#ifndef _BASEINFERENCE_H_
#define _BASEINFERENCE_H_
#include <fstream>
#include "NvInferPlugin.h"
#include "opencv2/opencv.hpp"
#include <sys/stat.h>
#include <io.h>
#include <process.h>
#include "NvInfer.h"
#define _S_IFMT 0xF000
#define S_IFMT _S_IFMT

#define _S_IFREG 0x8000
#define S_IFREG _S_IFREG

#define _S_IFDIR 0x4000
#define S_IFDIR _S_IFDIR

#define	S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define	S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define CHECK(call)                                   \
do                                                    \
{                                                     \
    const cudaError_t error_code = call;              \
    if (error_code != cudaSuccess)                    \
    {                                                 \
        printf("CUDA Error:\n");                      \
        printf("    File:       %s\n", __FILE__);     \
        printf("    Line:       %d\n", __LINE__);     \
        printf("    Error code: %d\n", error_code);   \
        printf("    Error text: %s\n",                \
            cudaGetErrorString(error_code));          \
        exit(1);                                      \
    }                                                 \
} while (0)

namespace my_Logger {
	class  Logger : public nvinfer1::ILogger
	{
	public:
		nvinfer1::ILogger::Severity reportableSeverity;

		explicit Logger(nvinfer1::ILogger::Severity severity = nvinfer1::ILogger::Severity::kINFO) :
			reportableSeverity(severity)
		{
		}

		void log(nvinfer1::ILogger::Severity severity, const char* msg) noexcept override
		{
			if (severity > reportableSeverity)
			{
				return;
			}
			switch (severity)
			{
			case nvinfer1::ILogger::Severity::kINTERNAL_ERROR:
				std::cerr << "INTERNAL_ERROR: ";
				break;
			case nvinfer1::ILogger::Severity::kERROR:
				std::cerr << "ERROR: ";
				break;
			case nvinfer1::ILogger::Severity::kWARNING:
				std::cerr << "WARNING: ";
				break;
			case nvinfer1::ILogger::Severity::kINFO:
				std::cerr << "INFO: ";
				break;
			default:
				std::cerr << "VERBOSE: ";
				break;
			}
			std::cerr << msg << std::endl;
		}
	};
}
inline int  get_size_by_dims(const nvinfer1::Dims& dims)
{
	int size = 1;
	for (int i = 0; i < dims.nbDims; i++)
	{
		size *= dims.d[i];
	}
	return size;
}

inline int  type_to_size(const nvinfer1::DataType& dataType)
{
	switch (dataType)
	{
	case nvinfer1::DataType::kFLOAT:
		return 4;
	case nvinfer1::DataType::kHALF:
		return 2;
	case nvinfer1::DataType::kINT32:
		return 4;
	case nvinfer1::DataType::kINT8:
		return 1;
	case nvinfer1::DataType::kBOOL:
		return 1;
	default:
		return 4;
	}
}

inline static float clamp(float val, float min, float max)
{
	return val > min ? (val < max ? val : max) : min;
}

inline bool  IsPathExist(const std::string& path)
{
	if (_access(path.c_str(), 0) == 0)
	{
		return true;
	}
	return false;
}

inline bool  IsFile(const std::string& path)
{
	if (!IsPathExist(path))
	{
		printf("%s:%d %s not exist\n", __FILE__, __LINE__, path.c_str());
		return false;
	}
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

inline bool  IsFolder(const std::string& path)
{
	if (!IsPathExist(path))
	{
		return false;
	}
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

namespace seg
{
	struct  Binding
	{
		size_t size = 1;
		size_t dsize = 1;
		nvinfer1::Dims dims;
		std::string name;
	};

	struct  Object
	{
		cv::Rect_<float> rect;
		int label = 0;
		float score = 0.0;
		float prob = 0.0;
		cv::Mat boxMask;
	};

	struct  PreParam
	{
		float ratio = 1.0f;
		float dw = 0.0f;
		float dh = 0.0f;
		float height = 0;
		float width = 0;
	};
}

using namespace seg;
class  beseinference
{
public:
	beseinference(const std::string& engine_file_path);
	~beseinference();

	void make_pipe(bool warmup = true);
	void copy_from_Mat(const cv::Mat& image);
	void copy_from_Mat(const cv::Mat& image, cv::Size& size);
	void letterbox(
		const cv::Mat& image,
		cv::Mat& out,
		cv::Size& size
	);
	void infer();
	void postprocess(
		std::vector<Object>& objs,
		float score_thres = 0.25f,
		float iou_thres = 0.65f,
		int topk = 100,
		int seg_channels = 32,
		int seg_h = 160,
		int seg_w = 160
	);
	static void draw_objects(
		const cv::Mat& image,
		cv::Mat& res,
		const std::vector<Object>& objs,
		const std::vector<std::string>& CLASS_NAMES,
		const std::vector<std::vector<unsigned int>>& COLORS,
		const std::vector<std::vector<unsigned int>>& MASK_COLORS
	);
	int num_bindings;
	int num_inputs = 0;
	int num_outputs = 0;
	std::vector<seg::Binding> input_bindings;
	std::vector<seg::Binding> output_bindings;
	std::vector<void*> host_ptrs;
	std::vector<void*> device_ptrs;

	PreParam pparam;
private:
	nvinfer1::ICudaEngine* engine = nullptr;
	nvinfer1::IRuntime* runtime = nullptr;
	nvinfer1::IExecutionContext* context = nullptr;
	cudaStream_t stream = nullptr;
	my_Logger::Logger gLogger{ nvinfer1::ILogger::Severity::kERROR };

};
#endif
