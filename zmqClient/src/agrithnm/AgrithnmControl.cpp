#include"AgrithnmControl.h"

std::vector<CircularQueue> AgriConCore::m_FrameRingBufferVecs;
std::vector<void*> AgriConCore::m_CamHandleList;
class ZmqMsgHandle* AgriConCore::m_zmq = nullptr;

AgriConCore::AgriConCore(std::string cfgPath)
{
    const auto config = cpptoml::parse_file(cfgPath);
    const auto cellTable = config->get_table("agri");
    std::string netConfigPath;
    if (cellTable) {
        const auto netpath = cellTable->get_as<std::string>("netConfigPath");
        netConfigPath = netpath->c_str();
    }
 //   /*
 //   * 相机初始化，初始化已连接相机
 //   */
	//CamInitial();
 //   /*
 //   * 初始化相机的线程，并且构造循环队列保存图片
 //   */
 //   CamFrameSaveToBuffersByMutilThread();
 //   /*
 //   * 初始化算法处理线程，n个图片存储循环队列->n个算法处理线程->n个算法处理线程做汇总
 //   */
 //   AgriRunTimeByMutilThread();
    /*
    * ZMQ interface 的初始化
    * 参数1 netConfigPath 为zmq接口 网络配置文件路径
    * 参数2 ServerOrClient = 0 表示该工程的只配置有服务端（同步）
    *                      = 1 表示该工程的只配置有客户端（同步）
    *                      = 2 表示该工程既配置有服务端也配置有客户端（异步）
    */
    ZmqInit(netConfigPath, 2);
}

AgriConCore::~AgriConCore()
{

}
void AgriConCore::ZmqInit(std::string netConfigPath, int ServerOrClient) {
    m_zmq = new ZmqMsgHandle(netConfigPath, ServerOrClient);
    m_zmq->run();

}
//遍历主机已连接相机设备，输出设备信息
void AgriConCore::CamInitial() {
    try {
        //获取枚举设备列表
        if (CMCam->EnumDevices(&pstDevList) == -1) {
            std::cout << "No camera found" << std::endl;
        }
        else {
            std::cout << "All camera found" << std::endl;
            for (unsigned int i = 0; i < pstDevList.nDeviceNum; i++)
            {
                printf("[device %d]:\n", i);
                MV_CC_DEVICE_INFO* pDeviceInfo = pstDevList.pDeviceInfo[i];
                if (NULL == pDeviceInfo)
                {
                    throw std::runtime_error("get device information error !!.");
                }
                CMCam->PrintDeviceInfo(pDeviceInfo);
            }
        }
        //分配设备句柄空间
        for (int i = 0; i < pstDevList.nDeviceNum; ++i) {
            m_CamHandleList.push_back(nullptr);
        }
        //获取已连接设备句柄并启动捕帧
        for (int nIndex = 0; nIndex < m_CamHandleList.size(); nIndex++) {
            // ch:选择设备并创建句柄 | en:Select device and create handle
            int nRet = MV_CC_CreateHandle(&m_CamHandleList[nIndex], pstDevList.pDeviceInfo[nIndex]);
            if (MV_OK != nRet)
            {
                printf("Create Handle fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Create Handle fail ! !!.");
            }

            // ch:打开设备 | en:Open device
            nRet = MV_CC_OpenDevice(m_CamHandleList[nIndex]);
            if (MV_OK != nRet)
            {
                printf("Open Device fail ! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Open Device fail !!.");
            }

            // ch:探测网络最佳包大小(只对GigE相机有效) | en:Detection network optimal package size(It only works for the GigE camera)
            if (pstDevList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
            {
                int nPacketSize = MV_CC_GetOptimalPacketSize(m_CamHandleList[nIndex]);
                if (nPacketSize > 0)
                {
                    nRet = MV_CC_SetIntValue(m_CamHandleList[nIndex], "GevSCPSPacketSize", nPacketSize);
                    if (nRet != MV_OK)
                    {
                        printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
                        throw std::runtime_error("Warning: Set Packet Size fail !!.");
                    }
                }
                else
                {
                    printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
                    throw std::runtime_error("Warning: Set Packet Size fail !!.");
                }
            }

            // ch:设置触发模式为off | en:Set trigger mode as off
            nRet = MV_CC_SetEnumValue(m_CamHandleList[nIndex], "TriggerMode", 0);
            if (MV_OK != nRet)
            {
                printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Set Trigger Mode fail !!!");
            }

            //ch:获取Enum型节点指定值的符号 | en:Get the symbol of the specified value of the Enum type node
            MVCC_ENUMVALUE stEnumValue = { 0 };
            MVCC_ENUMENTRY stEnumEntry = { 0 };
            nRet = MV_CC_GetEnumValue(m_CamHandleList[nIndex], "PixelFormat", &stEnumValue);
            if (MV_OK != nRet)
            {
                printf("Get PixelFormat's value fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Get PixelFormat's value fail !!!");
            }
            stEnumEntry.nValue = stEnumValue.nCurValue;
            nRet = MV_CC_GetEnumEntrySymbolic(m_CamHandleList[nIndex], "PixelFormat", &stEnumEntry);
            if (MV_OK != nRet)
            {
                printf("Get PixelFormat's symbol fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Get PixelFormat's value fail !!!");
            }
            else
            {
                printf("PixelFormat:%s\n", stEnumEntry.chSymbolic);
            }

            // ch:开始取流 | en:Start grab image
            nRet = MV_CC_StartGrabbing(m_CamHandleList[nIndex]);
            if (MV_OK != nRet)
            {
                printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Start Grabbing fail !!!");
            }
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }


}
//已生成相机，关闭设备，销毁相机句柄
void AgriConCore::CamDestroy() {
    try {
        for (int nIndex = 0; nIndex < m_CamHandleList.size(); nIndex++) {

            // ch:停止取流 | en:Stop grab image
            int nRet = MV_CC_StopGrabbing(m_CamHandleList[nIndex]);
            if (MV_OK != nRet)
            {
                printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Stop Grabbing fail !!!");
            }

            // ch:关闭设备 | Close device
            nRet = MV_CC_CloseDevice(m_CamHandleList[nIndex]);
            if (MV_OK != nRet)
            {
                printf("ClosDevice fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("ClosDevice fail ! !!!");
            }

            // ch:销毁句柄 | Destroy handle
            nRet = MV_CC_DestroyHandle(m_CamHandleList[nIndex]);
            if (MV_OK != nRet)
            {
                printf("Destroy Handle fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Destroy Handle fail ! !!!");
            }
        }
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }


}
void AgriConCore::CamFrameSaveToBuffer(int CamHandleOder) {
    MV_FRAME_OUT stOutFrame = {0};
    while (true) {
        //if (m_zmq->Agri_waitDataList.front().Mode == "softwave_trigger_on") { 
        //    std::cout<< "Mode: " << m_zmq->Agri_waitDataList.front().Mode << std::endl;
        //    std::cout << "Task: " << m_zmq->Agri_waitDataList.front().Task << std::endl;
        //    m_zmq->Agri_waitDataList.pop_front();
        //}
        //else if(m_zmq->Agri_waitDataList.front().Mode == "softwave_trigger_off"){
        //    int nRet = MV_CC_GetImageBuffer(m_CamHandleList[CamHandleOder], &stOutFrame, 1000);
        //    if (nRet == MV_OK)
        //    {
        //        cv::Size framSize(stOutFrame.stFrameInfo.nWidth, stOutFrame.stFrameInfo.nHeight);
        //        cv::Mat imgMat(framSize, CV_8UC3, stOutFrame.pBufAddr);
        //        m_FrameRingBufferVecs[CamHandleOder].enqueue(imgMat);//根据相机序号选择对应存储图像数据的循环队列
        //        nRet = MV_CC_FreeImageBuffer(m_CamHandleList[CamHandleOder], &stOutFrame);
        //        if (nRet != MV_OK)
        //        {
        //            printf("Free Image Buffer fail! nRet [0x%x]\n", nRet);
        //            break;
        //        }
        //    }
        //    else
        //    {
        //        printf("Get Image fail! nRet [0x%x]\n", nRet);
        //        break;
        //    }
        //}
        //else {
        //    std::cout<<"Mode configure error !!!" << std::endl;
        //    continue;
        //}

    }
    AgriConCore::CamDestroy();
}
//算法多线程和相机采图多线程中间需要一个环形缓存区ringbuffer实现采图和处理的异步，这样效率会更高一些
void AgriConCore::CamFrameSaveToBuffersByMutilThread() {
    
    const int numThreads = m_CamHandleList.size();
    //初始化ringbuffer 空间
    for (int i = 0; i < numThreads; ++i) {
        CircularQueue m_queue = CircularQueue(MaxRingBufferLen);//构建numThreads个环形队列
        m_FrameRingBufferVecs.push_back(m_queue);
    }
    // 创建并启动多个线程
    for (int i = 0; i < numThreads; ++i) {
        int variableValue = i; // 不同的变量值
        m_rev_threads.emplace_back(CamFrameSaveToBuffer, variableValue);
    }
    // 等待所有线程完成
    for (std::thread& thread : m_rev_threads) {
        thread.detach();
    }
}
/*
* 算法根据CamHandleOder来决定处理哪一个相机的图
*/
void AgriConCore::AgriRunTime(int CamHandleOder) {


}
void AgriConCore::AgriRunTimeByMutilThread() {
    const int numThreads = m_CamHandleList.size();
    //初始化ringbuffer 空间
    for (int i = 0; i < numThreads; ++i) {
        CircularQueue m_queue = CircularQueue(MaxRingBufferLen);//构建numThreads个环形队列
        m_FrameRingBufferVecs.push_back(m_queue);
    }
    // 创建并启动多个线程
    for (int i = 0; i < numThreads; ++i) {
        int variableValue = i; // 不同的变量值
        m_pro_threads.emplace_back(AgriRunTime, variableValue);
    }
    // 等待所有线程完成
    for (std::thread& thread : m_pro_threads) {
        thread.detach();
    }
}
void AgriConCore::ControlCore() {
    // 创建一个窗口
    cv::namedWindow("Video Stream", cv::WINDOW_NORMAL);

    while (true) {
        cv::Mat frame;
        if (!m_FrameRingBufferVecs[0].isEmpty()) {
            m_FrameRingBufferVecs[0].dequeue(frame);
        }
        else {
            continue;
        }
        // 检查是否成功读取帧
        if (frame.empty()) {
            std::cerr << "Error: Unable to read a frame." << std::endl;
            continue;
        }
        // 显示帧图像
        cv::imshow("Video Stream", frame);
        std::cout<<"cur size: "<< m_FrameRingBufferVecs[0].curSize()<<std::endl;
        // 等待一段时间，并检查是否按下了 'q' 键，如果按下则退出循环
        if (cv::waitKey(200) == 'q') {
            break;
        }
    }
    AgriConCore::CamDestroy();
    // 关闭窗口和摄像头
    cv::destroyAllWindows();

}
