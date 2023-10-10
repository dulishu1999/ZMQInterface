#ifndef _SEGRT_H_
#define _SEGRT_H_

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <vector>

#include "chrono"
#include "baseinference.h"
#include "opencv2/opencv.hpp"
#include "NvInfer.h"
#include "NvOnnxParser.h"
#include "NvinferRuntime.h"

#include "NvInferPlugin.h"


#include <windows.h>

#include <experimental/filesystem>

#define MrsDebug false;
#define OrsDebug false;

using namespace nvinfer1;
using namespace nvonnxparser;

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;
using duration = std::chrono::duration<float>;
using m_Arguments = std::unordered_multimap<std::string, std::string>;



struct  initialSettingConfig {
	int device = 0;
};
struct  MainRegionSegConfig {
	std::vector<std::string> MRS_ClassNames;
	std::vector<float> MRS_ClassConfidences;
	std::vector<int> MRS_ClassRegionsArea;
};
struct  ObjectRegionSegConfig {
	std::vector<std::string> ORS_ClassNames;
	std::vector<float> ORS_ClassConfidences;
	std::vector<int> ORS_ClassRegionsArea;
};

struct  ResultInfo
{
	cv::Mat drawedPic;
	std::string ngOrok;
	cv::Size mrsSize;
	std::vector<Object> objs;

};

class  mySegMachine
{
public:
	mySegMachine(initialSettingConfig config);
	~mySegMachine();
	ResultInfo PredictBox(cv::Mat frame, bool offlinetest);
	void writeMainRegionSegConfig(MainRegionSegConfig cfg);
	void writeObjectRegionSegConfig(ObjectRegionSegConfig cfg);
private:
	std::string my_logPath;
	cv::String my_offlineReadImgPath;
	cv::String my_offlineResultSavePath;
	cv::String my_exportEngineModelPath;
	cv::String my_importOnnxModelPath;
	cv::String my_MRS_EngineModelPath;
	cv::String my_ORS_EngineModelPath;

	std::vector<std::string> my_MRS_ClassNames;
	std::vector<float> my_MRS_ClassConfidences;
	std::vector<int> my_MRS_ClassRegionsArea;

	std::vector<std::string> my_ORS_ClassNames;
	std::vector<float> my_ORS_ClassConfidences;
	std::vector<int> my_ORS_ClassRegionsArea;

	std::vector<Object> activateSoreSifting(std::vector<Object> originObjs, std::vector<float> my_confi);
	std::vector<Object> activateAreaSifting(std::vector<Object> originObjs, std::vector<int> my_area);



	void setCudaDevice(int device);

	ResultInfo MainRegionSegBox(cv::Mat fram);
	ResultInfo ObjectRegionSegBox(cv::Mat fram);
	beseinference* MRS_inference;
	beseinference* ORS_inference;

	const std::vector<std::vector<unsigned int>> COLORS = {
	{ 0, 114, 189 }, { 217, 83, 25 }, { 237, 177, 32 },
	{ 126, 47, 142 }, { 119, 172, 48 }, { 77, 190, 238 },
	{ 162, 20, 47 }, { 76, 76, 76 }, { 153, 153, 153 },
	{ 255, 0, 0 }, { 255, 128, 0 }, { 191, 191, 0 },
	{ 0, 255, 0 }, { 0, 0, 255 }, { 170, 0, 255 },
	{ 85, 85, 0 }, { 85, 170, 0 }, { 85, 255, 0 },
	{ 170, 85, 0 }, { 170, 170, 0 }, { 170, 255, 0 },
	{ 255, 85, 0 }, { 255, 170, 0 }, { 255, 255, 0 },
	{ 0, 85, 128 }, { 0, 170, 128 }, { 0, 255, 128 },
	{ 85, 0, 128 }, { 85, 85, 128 }, { 85, 170, 128 },
	{ 85, 255, 128 }, { 170, 0, 128 }, { 170, 85, 128 },
	{ 170, 170, 128 }, { 170, 255, 128 }, { 255, 0, 128 },
	{ 255, 85, 128 }, { 255, 170, 128 }, { 255, 255, 128 },
	{ 0, 85, 255 }, { 0, 170, 255 }, { 0, 255, 255 },
	{ 85, 0, 255 }, { 85, 85, 255 }, { 85, 170, 255 },
	{ 85, 255, 255 }, { 170, 0, 255 }, { 170, 85, 255 },
	{ 170, 170, 255 }, { 170, 255, 255 }, { 255, 0, 255 },
	{ 255, 85, 255 }, { 255, 170, 255 }, { 85, 0, 0 },
	{ 128, 0, 0 }, { 170, 0, 0 }, { 212, 0, 0 },
	{ 255, 0, 0 }, { 0, 43, 0 }, { 0, 85, 0 },
	{ 0, 128, 0 }, { 0, 170, 0 }, { 0, 212, 0 },
	{ 0, 255, 0 }, { 0, 0, 43 }, { 0, 0, 85 },
	{ 0, 0, 128 }, { 0, 0, 170 }, { 0, 0, 212 },
	{ 0, 0, 255 }, { 0, 0, 0 }, { 36, 36, 36 },
	{ 73, 73, 73 }, { 109, 109, 109 }, { 146, 146, 146 },
	{ 182, 182, 182 }, { 219, 219, 219 }, { 0, 114, 189 },
	{ 80, 183, 189 }, { 128, 128, 0 }
	};
	const std::vector<std::vector<unsigned int>> MASK_COLORS = {
	{ 255, 56, 56 }, { 255, 157, 151 }, { 255, 112, 31 },
	{ 255, 178, 29 }, { 207, 210, 49 }, { 72, 249, 10 },
	{ 146, 204, 23 }, { 61, 219, 134 }, { 26, 147, 52 },
	{ 0, 212, 187 }, { 44, 153, 168 }, { 0, 194, 255 },
	{ 52, 69, 147 }, { 100, 115, 255 }, { 0, 24, 236 },
	{ 132, 56, 255 }, { 82, 0, 133 }, { 203, 56, 255 },
	{ 255, 149, 200 }, { 255, 55, 199 }
	};
};


#endif // !_SEGV8RT_H

