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
 //   * �����ʼ������ʼ�����������
 //   */
	//CamInitial();
 //   /*
 //   * ��ʼ��������̣߳����ҹ���ѭ�����б���ͼƬ
 //   */
 //   CamFrameSaveToBuffersByMutilThread();
 //   /*
 //   * ��ʼ���㷨�����̣߳�n��ͼƬ�洢ѭ������->n���㷨�����߳�->n���㷨�����߳�������
 //   */
 //   AgriRunTimeByMutilThread();
    /*
    * ZMQ interface �ĳ�ʼ��
    * ����1 netConfigPath Ϊzmq�ӿ� ���������ļ�·��
    * ����2 ServerOrClient = 0 ��ʾ�ù��̵�ֻ�����з���ˣ�ͬ����
    *                      = 1 ��ʾ�ù��̵�ֻ�����пͻ��ˣ�ͬ����
    *                      = 2 ��ʾ�ù��̼������з����Ҳ�����пͻ��ˣ��첽��
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
//������������������豸������豸��Ϣ
void AgriConCore::CamInitial() {
    try {
        //��ȡö���豸�б�
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
        //�����豸����ռ�
        for (int i = 0; i < pstDevList.nDeviceNum; ++i) {
            m_CamHandleList.push_back(nullptr);
        }
        //��ȡ�������豸�����������֡
        for (int nIndex = 0; nIndex < m_CamHandleList.size(); nIndex++) {
            // ch:ѡ���豸��������� | en:Select device and create handle
            int nRet = MV_CC_CreateHandle(&m_CamHandleList[nIndex], pstDevList.pDeviceInfo[nIndex]);
            if (MV_OK != nRet)
            {
                printf("Create Handle fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Create Handle fail ! !!.");
            }

            // ch:���豸 | en:Open device
            nRet = MV_CC_OpenDevice(m_CamHandleList[nIndex]);
            if (MV_OK != nRet)
            {
                printf("Open Device fail ! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Open Device fail !!.");
            }

            // ch:̽��������Ѱ���С(ֻ��GigE�����Ч) | en:Detection network optimal package size(It only works for the GigE camera)
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

            // ch:���ô���ģʽΪoff | en:Set trigger mode as off
            nRet = MV_CC_SetEnumValue(m_CamHandleList[nIndex], "TriggerMode", 0);
            if (MV_OK != nRet)
            {
                printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Set Trigger Mode fail !!!");
            }

            //ch:��ȡEnum�ͽڵ�ָ��ֵ�ķ��� | en:Get the symbol of the specified value of the Enum type node
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

            // ch:��ʼȡ�� | en:Start grab image
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
//������������ر��豸������������
void AgriConCore::CamDestroy() {
    try {
        for (int nIndex = 0; nIndex < m_CamHandleList.size(); nIndex++) {

            // ch:ֹͣȡ�� | en:Stop grab image
            int nRet = MV_CC_StopGrabbing(m_CamHandleList[nIndex]);
            if (MV_OK != nRet)
            {
                printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("Stop Grabbing fail !!!");
            }

            // ch:�ر��豸 | Close device
            nRet = MV_CC_CloseDevice(m_CamHandleList[nIndex]);
            if (MV_OK != nRet)
            {
                printf("ClosDevice fail! nRet [0x%x]\n", nRet);
                throw std::runtime_error("ClosDevice fail ! !!!");
            }

            // ch:���پ�� | Destroy handle
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
        //        m_FrameRingBufferVecs[CamHandleOder].enqueue(imgMat);//����������ѡ���Ӧ�洢ͼ�����ݵ�ѭ������
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
//�㷨���̺߳������ͼ���߳��м���Ҫһ�����λ�����ringbufferʵ�ֲ�ͼ�ʹ�����첽������Ч�ʻ����һЩ
void AgriConCore::CamFrameSaveToBuffersByMutilThread() {
    
    const int numThreads = m_CamHandleList.size();
    //��ʼ��ringbuffer �ռ�
    for (int i = 0; i < numThreads; ++i) {
        CircularQueue m_queue = CircularQueue(MaxRingBufferLen);//����numThreads�����ζ���
        m_FrameRingBufferVecs.push_back(m_queue);
    }
    // ��������������߳�
    for (int i = 0; i < numThreads; ++i) {
        int variableValue = i; // ��ͬ�ı���ֵ
        m_rev_threads.emplace_back(CamFrameSaveToBuffer, variableValue);
    }
    // �ȴ������߳����
    for (std::thread& thread : m_rev_threads) {
        thread.detach();
    }
}
/*
* �㷨����CamHandleOder������������һ�������ͼ
*/
void AgriConCore::AgriRunTime(int CamHandleOder) {


}
void AgriConCore::AgriRunTimeByMutilThread() {
    const int numThreads = m_CamHandleList.size();
    //��ʼ��ringbuffer �ռ�
    for (int i = 0; i < numThreads; ++i) {
        CircularQueue m_queue = CircularQueue(MaxRingBufferLen);//����numThreads�����ζ���
        m_FrameRingBufferVecs.push_back(m_queue);
    }
    // ��������������߳�
    for (int i = 0; i < numThreads; ++i) {
        int variableValue = i; // ��ͬ�ı���ֵ
        m_pro_threads.emplace_back(AgriRunTime, variableValue);
    }
    // �ȴ������߳����
    for (std::thread& thread : m_pro_threads) {
        thread.detach();
    }
}
void AgriConCore::ControlCore() {
    // ����һ������
    cv::namedWindow("Video Stream", cv::WINDOW_NORMAL);

    while (true) {
        cv::Mat frame;
        if (!m_FrameRingBufferVecs[0].isEmpty()) {
            m_FrameRingBufferVecs[0].dequeue(frame);
        }
        else {
            continue;
        }
        // ����Ƿ�ɹ���ȡ֡
        if (frame.empty()) {
            std::cerr << "Error: Unable to read a frame." << std::endl;
            continue;
        }
        // ��ʾ֡ͼ��
        cv::imshow("Video Stream", frame);
        std::cout<<"cur size: "<< m_FrameRingBufferVecs[0].curSize()<<std::endl;
        // �ȴ�һ��ʱ�䣬������Ƿ����� 'q' ��������������˳�ѭ��
        if (cv::waitKey(200) == 'q') {
            break;
        }
    }
    AgriConCore::CamDestroy();
    // �رմ��ں�����ͷ
    cv::destroyAllWindows();

}
