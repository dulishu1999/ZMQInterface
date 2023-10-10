#include<iostream>
#include "zmq/zmq_msg_handle.h"
#include"AgrithnmControl.h"
int main() {
	//std::string m_path = "E:\\my-work\\software-code\\zmqClientVS2019\\zmqClient\\images\\*.bmp";
 //   std::string m_Cfgpath = "E:\\my-work\\software-code\\zmqClientVS2019\\zmqClient\\config\\cell_MLA.toml";
 //   fp::ZmqMsgHandle* mZmq = new fp::ZmqMsgHandle(m_Cfgpath,1,1);
	//mZmq->run();
	//std:; string mu = "adadd";
	//cv::Mat blackImage(480, 640, CV_8UC3, cv::Scalar(0, 255, 124));
	//
	//while (true)
	//{
	//	if (mZmq->m_TaskData.empty()) {
	//		mZmq->sendTaskInstructions(mu, blackImage);
	//		std::this_thread::sleep_for(std::chrono::seconds(2));
	//	}
	//	
	//}
	AgriConCore* my = new AgriConCore("E:\\my-work\\software-code\\zmqClientVS2019\\zmqClient\\config\\agri_config.toml");
	//my->ControlCore();
	while (1) {}
	return 0;
}
