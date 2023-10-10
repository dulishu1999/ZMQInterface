#include "segRT.h"

mySegMachine::mySegMachine(initialSettingConfig config)
{
    auto fpath = std::experimental::filesystem::current_path();
    std::string path = fpath.parent_path().string() + "\\";
	setCudaDevice(config.device);
	this->my_offlineReadImgPath = path+"srcImgs\\*.bmp";
    this->my_offlineResultSavePath = path+"results\\";
    this->my_exportEngineModelPath = path+"model\\best.bin";
    this->my_importOnnxModelPath = path+"model\\best.onnx";
    this->my_MRS_EngineModelPath = path+"model\\mrsbest.bin";
    this->my_ORS_EngineModelPath = path+"model\\orsbest.bin";

    this->MRS_inference = new beseinference(this->my_MRS_EngineModelPath);
    this->MRS_inference->make_pipe(true);
    this->ORS_inference = new beseinference(this->my_ORS_EngineModelPath);
    this->ORS_inference->make_pipe(true); 
}  

mySegMachine::~mySegMachine()
{
	delete MRS_inference;
    delete ORS_inference;
}
void mySegMachine::writeMainRegionSegConfig(MainRegionSegConfig cfg) {
    this->my_MRS_ClassConfidences = cfg.MRS_ClassConfidences;
    this->my_MRS_ClassNames = cfg.MRS_ClassNames;
    this->my_MRS_ClassRegionsArea = cfg.MRS_ClassRegionsArea;
    if (this->my_MRS_ClassConfidences.empty() 
        || this->my_MRS_ClassNames.empty() 
        || this->my_MRS_ClassNames.size() != this->my_MRS_ClassConfidences.size()) {
        throw " MRS: The class and confidence are misconfigured ! ! !";
    }
}
void mySegMachine::writeObjectRegionSegConfig(ObjectRegionSegConfig cfg) {
    this->my_ORS_ClassConfidences = cfg.ORS_ClassConfidences;
    this->my_ORS_ClassNames = cfg.ORS_ClassNames;
    this->my_ORS_ClassRegionsArea = cfg.ORS_ClassRegionsArea;
    if (this->my_ORS_ClassConfidences.empty()
        || this->my_ORS_ClassNames.empty()
        || this->my_ORS_ClassNames.size() != this->my_ORS_ClassConfidences.size()) {
        throw " ORS: The class and confidence are misconfigured ! ! !";
    }
}

void mySegMachine::setCudaDevice(int device)
{
	cudaError_t ret = cudaSetDevice(device);
	if (ret != 0)
	{
        
       std::cout<<"Cuda failure: "<< cudaGetErrorString(ret)<<std::endl;
		abort();
	}

	cudaDeviceProp properties;
	ret = cudaGetDeviceProperties(&properties, device);
	if (ret != 0)
	{
		std::cout<<"Cuda failure: "<<cudaGetErrorString(ret)<<std::endl;
		abort();
	}
   std::cout<<"=== Device Information ==="<<std::endl;
   std::cout << "Selected Device: " << properties.name << std::endl;
   std::cout<<"Compute Capability: "<<properties.major<<"."<<properties.minor<<std::endl;
   std::cout<<"SMs: "<<properties.multiProcessorCount<<std::endl;
   std::cout<<"Compute Clock Rate: "<<properties.clockRate / 1000000.0F<<" GHz"<<std::endl;
   std::cout<<"Device Global Memory: "<<(properties.totalGlobalMem >> 20)<<" MiB"<<std::endl;
   std::cout<<"Shared Memory per SM: "<<(properties.sharedMemPerMultiprocessor >> 10)<< " KiB"<<std::endl;
   std::cout << "Memory Bus Width: " << properties.memoryBusWidth << " bits" << " (ECC " << (properties.ECCEnabled != 0 ? "enabled" : "disabled") << ")" << std::endl;
   std::cout<<"Memory Clock Rate: "<<properties.memoryClockRate / 1000000.0F<< " GHz"<<std::endl;
}

std::vector<Object> mySegMachine::activateSoreSifting(std::vector<Object> originObjs, std::vector<float> my_confi) {
	std::vector<Object> siftedObjs;
	int index;
	for (int i = 0; i < originObjs.size(); i++) {
		index = (int)originObjs[i].label;
		if (originObjs[i].prob> my_confi[index]) {
			siftedObjs.push_back(originObjs[i]);
		}
	}
	return siftedObjs;
}
std::vector<Object> mySegMachine::activateAreaSifting(std::vector<Object> originObjs, std::vector<int> my_area) {
	std::vector<Object> siftedObjs;
    std::vector<Object> zm_tusun_siftedObjs;
    //std::vector<Object> zm_zangwu_siftedObjs;
	int index;
	for (int i = 0; i < originObjs.size(); i++) {
		index = originObjs[i].label;
        std::vector<cv::Point> nonZeroPixs;//非零向量坐标数量
		cv::findNonZero(originObjs[i].boxMask, nonZeroPixs);
		int nonZeroPixNum = nonZeroPixs.size();
		nonZeroPixs.clear();
		if (nonZeroPixNum > my_area[index]) {
			siftedObjs.push_back(originObjs[i]);
		}
        else {
            if (this->my_ORS_ClassNames[originObjs[i].label] == "zm_tusun") {
                zm_tusun_siftedObjs.push_back(originObjs[i]);
            }
            //if (this->my_ORS_ClassNames[originObjs[i].label] == "zm_zangwu") { 
            //    zm_zangwu_siftedObjs.push_back(originObjs[i]);
            //}
        }
	}
    if (zm_tusun_siftedObjs.size()>2) {
        for (auto ks : zm_tusun_siftedObjs) {
            siftedObjs.push_back(ks);
        }
    }
    //if (zm_zangwu_siftedObjs.size() > 2) {
    //    for (auto ks : zm_zangwu_siftedObjs) {
    //        siftedObjs.push_back(ks);
    //    }
    //}
    zm_tusun_siftedObjs.clear();
    //zm_zangwu_siftedObjs.clear();
	return siftedObjs;
}
ResultInfo mySegMachine::PredictBox(cv::Mat frame, bool offlinetest) { 
    ResultInfo mRi;
    ResultInfo oRi;
    if (offlinetest == true) {
        std::vector<cv::String> fn;
        cv::glob(my_offlineReadImgPath, fn, false);
        for (int i = 0; i < fn.size(); i++) {
            cv::String picname = fn[i];
            cv::Mat frame = cv::imread(picname, cv::IMREAD_UNCHANGED);
            if (frame.channels() == 1) {
                cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);
            }
            cv::Mat mainRegion;
            std::cout<<"Frame order: "<< std::to_string(i)<<" Fame name : "<<picname.substr(picname.find_last_of("\\") + 1, picname.size() - 1)<<std::endl;
            auto start1 = std::chrono::system_clock::now();
            mRi = MainRegionSegBox(frame);
            if (mRi.ngOrok == "detected") {
                frame(mRi.objs[0].rect).copyTo(mainRegion, mRi.objs[0].boxMask);
                //cv::Mat nonZerosPoints;
                //cv::findNonZero(mRi.objs[0].boxMask, nonZerosPoints);
                ////返回最小外接矩形
                //cv::RotatedRect rotRect = cv::minAreaRect(nonZerosPoints);
                //mRi.mrsSize = rotRect.size;
                std::cout << "The principal component segmentation was successful......" << std::endl;
            }
            else if (mRi.ngOrok == "onemore") {
               std::cout<<"There are multiple main component segmentation regions. Suggestion: Please check if there are multiple parts in the field of view"<<std::endl;
               std::cout << "The principal component segmentation failed......" << std::endl;
               continue;
            }
            else if (mRi.ngOrok == "empty") {
               std::cout<<"There are no parts present in the field of view. Suggestion: Please check if there are any parts present in the field of view"<<std::endl;
               std::cout << "The principal component segmentation failed......" << std::endl;
               continue;
            }  
            //oRi = ObjectRegionSegBox(mainRegion);
            //oRi.drawedPic.copyTo(frame(mRi.objs[0].rect), mRi.objs[0].boxMask);
            //auto end1 = std::chrono::system_clock::now();
            //auto tc1 = (double)std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count() / 1000.;
            //std::cout << "TOTAL TIME >> the time cost of proceess is: " << tc1 << " ms" << std::endl;
            //oRi.mrsSize = mRi.mrsSize;
            //if (oRi.ngOrok == "ok") {
            //    cv::putText(frame, "OK", cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
            //}
            //else if (oRi.ngOrok == "ng") {
            //    cv::putText(frame, "NG", cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
            //}
            //int yStep = 100;
            //for (auto ob: oRi.objs) {
            //    if (yStep + 50 > frame.rows) {
            //        continue;
            //    }
            //    cv::Mat ors_nonZerosPoints;
            //    cv::findNonZero(ob.boxMask, ors_nonZerosPoints);
            //    //返回最小外接矩形
            //    cv::RotatedRect rotRect = cv::minAreaRect(ors_nonZerosPoints);
            //    cv::Size orsSize = rotRect.size;
            //    cv::putText(frame, this->my_ORS_ClassNames[ob.label]+": ", cv::Point(50, yStep ), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
            //    cv::putText(frame, "height: "+std::to_string(orsSize.height)+" width: "+ std::to_string(orsSize.width) + " area: " + std::to_string(ors_nonZerosPoints.size().height)+ " score: " + std::to_string(ob.prob), cv::Point(50, yStep + 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
            //    yStep += 100;
            //}
            cv::imwrite(my_offlineResultSavePath + picname.substr(picname.find_last_of("\\") + 1, picname.size() - 1), mainRegion);
        }
    }
    else if (offlinetest == false) {
        if (frame.channels() == 1) {
            cv::cvtColor(frame, frame, cv::COLOR_GRAY2BGR);
        }
        cv::Mat mainRegion;
        mRi = MainRegionSegBox(frame);
        if (mRi.ngOrok == "detected") {
            frame(mRi.objs[0].rect).copyTo(mainRegion, mRi.objs[0].boxMask);
            //cv::Mat nonZerosPoints;
            //cv::findNonZero(mRi.objs[0].boxMask, nonZerosPoints);
            ////返回最小外接矩形
            //cv::RotatedRect rotRect = cv::minAreaRect(nonZerosPoints);
            //mRi.mrsSize = rotRect.size;
            std::cout << "The principal component segmentation was successful......" << std::endl;
        }
        else if (mRi.ngOrok == "onemore") {
            std::cout << "There are multiple main component segmentation regions. Suggestion: Please check if there are multiple parts in the field of view" << std::endl;
            std::cout << "The principal component segmentation failed......" << std::endl;
            return mRi;
        }
        else if (mRi.ngOrok == "empty") {
            std::cout << "There are no parts present in the field of view. Suggestion: Please check if there are any parts present in the field of view" << std::endl;
            std::cout << "The principal component segmentation failed......" << std::endl;
            return mRi;
        }
        oRi = ObjectRegionSegBox(mainRegion);
        oRi.drawedPic.copyTo(frame(mRi.objs[0].rect), mRi.objs[0].boxMask);
        //oRi.mrsSize = mRi.mrsSize;
        //if (oRi.ngOrok == "ok") {
        //    cv::putText(frame, "OK", cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        //}
        //else if (oRi.ngOrok == "ng") {
        //    cv::putText(frame, "NG", cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        //}
        //int yStep = 100;
        //for (auto ob : oRi.objs) {
        //    if (yStep + 50 > frame.rows) {
        //        continue;
        //    }
        //    cv::Mat ors_nonZerosPoints;
        //    cv::findNonZero(ob.boxMask, ors_nonZerosPoints);
        //    //返回最小外接矩形
        //    cv::RotatedRect rotRect = cv::minAreaRect(ors_nonZerosPoints);
        //    cv::Size orsSize = rotRect.size;
        //    cv::putText(frame, this->my_ORS_ClassNames[ob.label] + ": ", cv::Point(50, yStep), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2);
        //    cv::putText(frame, "height: " + std::to_string(orsSize.height) + " width: " + std::to_string(orsSize.width) + " area: " + std::to_string(ors_nonZerosPoints.size().height), cv::Point(50, yStep + 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
        //    yStep += 100;
        //}
        oRi.drawedPic = frame.clone();
    }
    return oRi;
}
ResultInfo mySegMachine::MainRegionSegBox(cv::Mat frame) {
	cv::Mat res;
	ResultInfo Ri;
	cv::Size size = cv::Size{ 640, 640 };
	int topk = 100; 
	int seg_h = 160;
	int seg_w = 160;
	int seg_channels = 32;
	float score_thres = 0.3f;
	float iou_thres = 0.35f;

	std::vector<Object> objs;
	auto start1 = std::chrono::system_clock::now();
    this->MRS_inference->copy_from_Mat(frame, size);
	auto end1 = std::chrono::system_clock::now();
	auto tc1 = (double)std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count() / 1000.;
	std::cout<<"MRS >> the time cost of Preprocess is: "<< tc1 <<" ms"<<std::endl;
	auto start2 = std::chrono::system_clock::now();
    this->MRS_inference->infer();
	auto end2 = std::chrono::system_clock::now();
	auto tc2 = (double)std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count() / 1000.;
	std::cout<<"MRS >> the time cost of inference is: "<< tc2 <<" ms"<<std::endl;
	auto start3 = std::chrono::system_clock::now();
    this->MRS_inference->postprocess(objs, score_thres, iou_thres, topk, seg_channels, seg_h, seg_w);
	auto end3 = std::chrono::system_clock::now();
	auto tc3 = (double)std::chrono::duration_cast<std::chrono::microseconds>(end3 - start3).count() / 1000.;
	std::cout<<"MRS >> the time cost of Postprocess is: "<< tc3 <<" ms"<< std::endl;
	if (objs.empty()) {//未检出主成分区，处理成异常
		Ri.drawedPic = frame.clone();
		Ri.ngOrok = "empty";
		Ri.objs = objs;
		return Ri;
	}
	std::vector<Object> siftingObjs = activateSoreSifting(objs, this->my_MRS_ClassConfidences);
	if (siftingObjs.empty()) {//未检出主成分区，处理成异常
		Ri.drawedPic = frame.clone();
		Ri.ngOrok = "empty";
		Ri.objs = objs;
		return Ri;
	}
    else if (siftingObjs.size() > 1) {//主成分区存在多个，处理成异常
        this->MRS_inference->draw_objects(frame, res, siftingObjs, this->my_MRS_ClassNames, this->COLORS, this->MASK_COLORS);
        Ri.drawedPic = frame.clone();
        Ri.ngOrok = "onemore";
        Ri.objs = objs;  
        return Ri;
    }
	else {
        this->MRS_inference->draw_objects(frame, res, siftingObjs, this->my_MRS_ClassNames, this->COLORS, this->MASK_COLORS); 
		Ri.drawedPic = res.clone();
		Ri.ngOrok = "detected";
		Ri.objs = objs;
		return Ri;
	}
}
ResultInfo mySegMachine::ObjectRegionSegBox(cv::Mat frame) {
    cv::Mat res;
    ResultInfo Ri;
    cv::Size size = cv::Size{ 640, 640 };
    int topk = 100;
    int seg_h = 160;
    int seg_w = 160;
    int seg_channels = 32;
    float score_thres = 0.25f;
    float iou_thres = 0.65f;

    std::vector<Object> objs;
    auto start1 = std::chrono::system_clock::now();
    this->ORS_inference->copy_from_Mat(frame, size);
    auto end1 = std::chrono::system_clock::now();
    auto tc1 = (double)std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count() / 1000.;
   std::cout<<"ORS >> the time cost of Preprocess is: "<< tc1 <<" ms"<<std::endl;
    auto start2 = std::chrono::system_clock::now();
    this->ORS_inference->infer();
    auto end2 = std::chrono::system_clock::now();
    auto tc2 = (double)std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2).count() / 1000.;
   std::cout<<"ORS >> the time cost of inference is : "<< tc2 <<" ms"<<std::endl;
    auto start3 = std::chrono::system_clock::now();
    this->ORS_inference->postprocess(objs, score_thres, iou_thres, topk, seg_channels, seg_h, seg_w);
    auto end3 = std::chrono::system_clock::now();
    auto tc3 = (double)std::chrono::duration_cast<std::chrono::microseconds>(end3 - start3).count() / 1000.;
   std::cout <<"ORS >> the time cost of Postprocess is: " << tc3 << " ms" << std::endl;
    if (objs.empty()) {
        Ri.drawedPic = frame.clone();
        Ri.ngOrok = "ok";
        return Ri;
    }
    std::vector<Object> siftingObjs_score = activateSoreSifting(objs, this->my_ORS_ClassConfidences);//分数筛
    if (siftingObjs_score.empty()) {//没有检出一个瑕疵
        Ri.drawedPic = frame.clone();
        Ri.ngOrok = "ok";
        return Ri;
    }
    std::vector<Object> siftingObjs_area = activateAreaSifting(siftingObjs_score, this->my_ORS_ClassRegionsArea);//面积筛
    if (siftingObjs_area.empty()) {//没有检出一个瑕疵
        Ri.drawedPic = frame.clone();
        Ri.ngOrok = "ok";
        return Ri;
    }
    else {
        this->ORS_inference->draw_objects(frame, res, siftingObjs_area, this->my_ORS_ClassNames, this->COLORS, this->MASK_COLORS);
        Ri.drawedPic = res.clone();
        Ri.ngOrok = "ng";
        Ri.objs = siftingObjs_area;
        return Ri;
    }
}



