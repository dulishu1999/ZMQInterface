#include"baseinference.h"
beseinference::beseinference(const std::string& engine_file_path)
{
	std::ifstream file(engine_file_path, std::ios::binary);
	assert(file.good());
	file.seekg(0, std::ios::end);//指针偏移到end获取模型字节量大小
	auto size = file.tellg();
	file.seekg(0, std::ios::beg);//指针偏移到beg获取模型序列初始位置
	char* trtModelStream = new char[size];//按char类型字节读取
	assert(trtModelStream);
	file.read(trtModelStream, size);//读取模型
	file.close();//关闭文件
	initLibNvInferPlugins(&this->gLogger, "");//初始化自定义插件
	this->runtime = nvinfer1::createInferRuntime(this->gLogger);
	assert(this->runtime != nullptr);

	this->engine = this->runtime->deserializeCudaEngine(trtModelStream, size);//反序列化转转成engine
	assert(this->engine != nullptr);
	delete[] trtModelStream;//析构trtModelStream对象，释放内存
	//Engine的运行需要一个运行时环境，createExecutionContext()方法为相应的ICudaEngine生成
	//一个IExecutionContext类型的运行环境context
	this->context = this->engine->createExecutionContext();//调用engine创建可执行上下文
	assert(this->context != nullptr);//判断上下文是否为空
	cudaStreamCreate(&this->stream);
	this->num_bindings = this->engine->getNbBindings(); //getNbBindings()获取网络输入输出数量
	for (int i = 0; i < this->num_bindings; ++i)
	{
		Binding binding;
		nvinfer1::Dims dims;
		nvinfer1::DataType dtype = this->engine->getBindingDataType(i);
		std::string name = this->engine->getBindingName(i);
		binding.name = name;
		binding.dsize = type_to_size(dtype);

		bool IsInput = engine->bindingIsInput(i);
		if (IsInput)
		{
			this->num_inputs += 1;
			dims = this->engine->getProfileDimensions(i,0,nvinfer1::OptProfileSelector::kMAX);
			binding.size = get_size_by_dims(dims);
			binding.dims = dims;
			this->input_bindings.push_back(binding);
			// set max opt shape
			this->context->setBindingDimensions(i, dims);

		}
		else
		{
			dims = this->context->getBindingDimensions(i);
			binding.size = get_size_by_dims(dims);
			binding.dims = dims;
			this->output_bindings.push_back(binding);
			this->num_outputs += 1;
		}
	}

}

beseinference::~beseinference()
{
	this->context->destroy();
	this->engine->destroy();
	this->runtime->destroy();
	cudaStreamDestroy(this->stream);
	for (auto& ptr : this->device_ptrs)
	{
		CHECK(cudaFree(ptr));
	}

	for (auto& ptr : this->host_ptrs)
	{
		CHECK(cudaFreeHost(ptr)); 
	}
}

void beseinference::make_pipe(bool warmup)
{

	for (auto& bindings : this->input_bindings)
	{
		void* d_ptr;
		CHECK(cudaMallocAsync(&d_ptr,bindings.size * bindings.dsize,this->stream));
		this->device_ptrs.push_back(d_ptr);
	}

	for (auto& bindings : this->output_bindings)
	{
		void* d_ptr, * h_ptr;
		size_t size = bindings.size * bindings.dsize;
		CHECK(cudaMallocAsync(
			&d_ptr,
			size,
			this->stream)
		);
		CHECK(cudaHostAlloc(
			&h_ptr,
			size,
			0)
		);
		this->device_ptrs.push_back(d_ptr);
		this->host_ptrs.push_back(h_ptr);
	}

	if (warmup)
	{
		for (int i = 0; i < 10; i++)
		{
			for (auto& bindings : this->input_bindings)
			{
				size_t size = bindings.size * bindings.dsize;
				void* h_ptr = malloc(size);
				memset(h_ptr, 0, size);
				CHECK(cudaMemcpyAsync(
					this->device_ptrs[0],
					h_ptr,
					size,
					cudaMemcpyHostToDevice,
					this->stream)
				);
				free(h_ptr);
			}
			this->infer();
		}
		std::cout << "model warmup 10 times\n"<< std::endl;

	}
}

void beseinference::letterbox(const cv::Mat& image,cv::Mat& out,cv::Size& size){
	const float inp_h = size.height;
	const float inp_w = size.width;
	float height = image.rows;
	float width = image.cols;

	float r = std::min(inp_h / height, inp_w / width);
	int padw = std::round(width * r);
	int padh = std::round(height * r);

	cv::Mat tmp;
	if ((int)width != padw || (int)height != padh)
	{
		cv::resize(image,tmp,cv::Size(padw, padh));
	}
	else
	{
		tmp = image.clone();
	}

	float dw = inp_w - padw;
	float dh = inp_h - padh;

	dw /= 2.0f;
	dh /= 2.0f;
	int top = int(std::round(dh - 0.1f));
	int bottom = int(std::round(dh + 0.1f));
	int left = int(std::round(dw - 0.1f));
	int right = int(std::round(dw + 0.1f));

	cv::copyMakeBorder(tmp,tmp,top,bottom,left,right,cv::BORDER_CONSTANT, { 114, 114, 114 });

	cv::dnn::blobFromImage(tmp,out,1 / 255.f,cv::Size(),cv::Scalar(0, 0, 0),true,false,CV_32F);
	this->pparam.ratio = 1 / r;
	this->pparam.dw = dw;
	this->pparam.dh = dh;
	this->pparam.height = height;
	this->pparam.width = width;
}

void beseinference::copy_from_Mat(const cv::Mat& image)
{
	cv::Mat nchw;
	auto& in_binding = this->input_bindings[0];
	auto width = in_binding.dims.d[3];
	auto height = in_binding.dims.d[2];
	cv::Size size{ width, height };
	this->letterbox(image,nchw,size);

	this->context->setBindingDimensions(0,nvinfer1::Dims{4,{ 1, 3, height, width }});
	CHECK(cudaMemcpyAsync(this->device_ptrs[0],nchw.ptr<float>(),nchw.total() * nchw.elemSize(),cudaMemcpyHostToDevice,this->stream));
}

void beseinference::copy_from_Mat(const cv::Mat& image, cv::Size& size)
{
	cv::Mat nchw;
	this->letterbox(image,nchw,size);
	this->context->setBindingDimensions(0,nvinfer1::Dims{ 4,{ 1, 3, size.height, size.width }});
	CHECK(cudaMemcpyAsync(this->device_ptrs[0],nchw.ptr<float>(),nchw.total() * nchw.elemSize(),cudaMemcpyHostToDevice,this->stream));
}

void beseinference::infer()
{

	this->context->enqueueV2(this->device_ptrs.data(),this->stream,nullptr);
	for (int i = 0; i < this->num_outputs; i++)
	{
		size_t osize = this->output_bindings[i].size * this->output_bindings[i].dsize;
		CHECK(cudaMemcpyAsync(this->host_ptrs[i],this->device_ptrs[i + this->num_inputs],osize,cudaMemcpyDeviceToHost,this->stream));

	}
	cudaStreamSynchronize(this->stream);

}

void beseinference::postprocess(std::vector<Object>& objs,float score_thres,float iou_thres,int topk,int seg_channels,int seg_h,int seg_w){
	objs.clear();
	auto input_h = this->input_bindings[0].dims.d[2];
	auto input_w = this->input_bindings[0].dims.d[3];
	int num_channels, num_anchors, num_classes;
	bool flag = false;
	int bid;
	int bcnt = -1;
	for (auto& o : this->output_bindings)
	{
		bcnt += 1;
		if (o.dims.nbDims == 3)
		{
			num_channels = o.dims.d[1];
			num_anchors = o.dims.d[2];
			flag = true;
			bid = bcnt;
		}
	}
	assert(flag);
	num_classes = num_channels - seg_channels - 4;

	auto& dw = this->pparam.dw;
	auto& dh = this->pparam.dh;
	auto& width = this->pparam.width;
	auto& height = this->pparam.height;
	auto& ratio = this->pparam.ratio;

	cv::Mat output = cv::Mat(num_channels, num_anchors, CV_32F,static_cast<float*>(this->host_ptrs[bid]));
	output = output.t();

	cv::Mat protos = cv::Mat(seg_channels, seg_h * seg_w, CV_32F,static_cast<float*>(this->host_ptrs[1 - bid]));

	std::vector<int> labels;
	std::vector<float> scores;
	std::vector<cv::Rect> bboxes;
	std::vector<cv::Mat> mask_confs;
	std::vector<int> indices;

	for (int i = 0; i < num_anchors; i++)
	{
		auto row_ptr = output.row(i).ptr<float>();
		auto bboxes_ptr = row_ptr;
		auto scores_ptr = row_ptr + 4;
		auto mask_confs_ptr = row_ptr + 4 + num_classes;
		auto max_s_ptr = std::max_element(scores_ptr, scores_ptr + num_classes);
		float score = *max_s_ptr;
		if (score > score_thres)
		{
			float x = *bboxes_ptr++ - dw;
			float y = *bboxes_ptr++ - dh;
			float w = *bboxes_ptr++;
			float h = *bboxes_ptr;

			float x0 = clamp((x - 0.5f * w) * ratio, 0.f, width);
			float y0 = clamp((y - 0.5f * h) * ratio, 0.f, height);
			float x1 = clamp((x + 0.5f * w) * ratio, 0.f, width);
			float y1 = clamp((y + 0.5f * h) * ratio, 0.f, height);

			int label = max_s_ptr - scores_ptr;
			cv::Rect_<float> bbox;
			bbox.x = x0;
			bbox.y = y0;
			bbox.width = x1 - x0;
			bbox.height = y1 - y0;

			cv::Mat mask_conf = cv::Mat(1, seg_channels, CV_32F, mask_confs_ptr);

			bboxes.push_back(bbox);
			labels.push_back(label);
			scores.push_back(score);
			mask_confs.push_back(mask_conf);
		}
	}

#if defined(BATCHED_NMS)
	cv::dnn::NMSBoxesBatched(bboxes,scores,labels,score_thres,iou_thres,indices);
#else
	cv::dnn::NMSBoxes(bboxes,scores,score_thres,iou_thres,indices);
#endif

	cv::Mat masks;
	int cnt = 0;
	for (auto& i : indices)
	{
		if (cnt >= topk)
		{
			break;
		}
		cv::Rect tmp = bboxes[i];
		Object obj;
		obj.label = labels[i];
		obj.rect = tmp;
		obj.prob = scores[i];
		masks.push_back(mask_confs[i]);
		objs.push_back(obj);
		cnt += 1;
	}

	if (masks.empty())
	{
		//masks is empty
	}
	else
	{
		cv::Mat matmulRes = (masks * protos).t();
		cv::Mat maskMat = matmulRes.reshape(indices.size(), { seg_w, seg_h });

		std::vector<cv::Mat> maskChannels;
		cv::split(maskMat, maskChannels);
		int scale_dw = dw / input_w * seg_w;
		int scale_dh = dh / input_h * seg_h;

		cv::Rect roi(scale_dw,scale_dh,seg_w - 2 * scale_dw,seg_h - 2 * scale_dh);
		for (int i = 0; i < indices.size(); i++)
		{
			cv::Mat dest, mask;
			cv::exp(-maskChannels[i], dest);
			dest = 1.0 / (1.0 + dest);
			dest = dest(roi);
			cv::resize(dest,mask,cv::Size((int)width, (int)height),cv::INTER_LINEAR);
			if (objs[i].rect.height * objs[i].rect.width == 0)continue;
			objs[i].boxMask = mask(objs[i].rect) > 0.5f;
		}
	}

}

void beseinference::draw_objects(const cv::Mat& image,
	cv::Mat& res,
	const std::vector<Object>& objs,
	const std::vector<std::string>& CLASS_NAMES,
	const std::vector<std::vector<unsigned int>>& COLORS,
	const std::vector<std::vector<unsigned int>>& MASK_COLORS
) {
	res = image.clone();
	cv::Mat mask = image.clone();
	for (auto& obj : objs)
	{
		int idx = obj.label;
		cv::Scalar color = cv::Scalar(COLORS[idx][0],COLORS[idx][1],COLORS[idx][2]);
		cv::Scalar mask_color = cv::Scalar(MASK_COLORS[idx % 20][0],MASK_COLORS[idx % 20][1],MASK_COLORS[idx % 20][2]);
		cv::rectangle(res,obj.rect,color,5);
        char text[256];
		sprintf_s(text,"%s %.1f%%",CLASS_NAMES[idx].c_str(),obj.prob * 100);
		mask(obj.rect).setTo(mask_color, obj.boxMask);//利用mask进行图像融合
		int baseLine = 0;
		cv::Size label_size = cv::getTextSize(text,cv::FONT_HERSHEY_SIMPLEX,0.4,1,&baseLine);
		int x = (int)obj.rect.x;
		int y = (int)obj.rect.y + 1;
		if (y > res.rows) {
			y = res.rows;
		}
		cv::rectangle(res,cv::Rect(x, y, label_size.width, label_size.height + baseLine),{ 0, 0, 255 },-1);
		cv::putText(res,text,cv::Point(x, y + label_size.height),cv::FONT_HERSHEY_SIMPLEX,0.4,{ 255, 255, 255 },1);
	}
	cv::addWeighted(res,0.5,mask,0.8,1,res);
}