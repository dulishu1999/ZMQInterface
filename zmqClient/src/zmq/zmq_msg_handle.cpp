#include "zmq/zmq_msg_handle.h"


using namespace fp;

static void SplitString(const std::string& str, std::vector<std::string>& vec, const std::string& ch )
{
    std::string::size_type pos1, pos2;
    pos2 = str.find( ch );
    pos1 = 0;
    while (std::string::npos != pos2)
    {
        vec.push_back(str.substr(pos1, pos2 - pos1));

        pos1 = pos2 + ch.size();
        pos2 = str.find( ch, pos1);
    }
    if (pos1 != str.length())
        vec.push_back(str.substr(pos1));
}
// Base64编码函数
std::string base64Encode(const std::vector<uint8_t>& data) {
    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int i = 0;
    int j = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];

    for (auto& byte : data) {
        char_array_3[i++] = byte;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2];

            for (i = 0; i < 4; i++) {
                result += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }

    if (i > 0) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++) {
            result += base64_chars[char_array_4[j]];
        }

        while (i++ < 3) {
            result += '=';
        }
    }

    return result;
}
// Base64解码函数
std::vector<uint8_t> base64Decode(const std::string& encoded) {
    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<uint8_t> result;
    int i = 0;
    int in_len = static_cast<int>(encoded.size());
    int j = 0;
    uint8_t char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded[j] != '=') && (isalnum(encoded[j]) || (encoded[j] == '+') || (encoded[j] == '/'))) {
        char_array_4[i++] = encoded[j];
        j++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = static_cast<uint8_t>(base64_chars.find(char_array_4[i]));
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; i < 3; i++) {
                result.push_back(char_array_3[i]);
            }
            i = 0;
        }
    }

    if (i > 0) {
        for (int k = 0; k < i; k++) {
            char_array_4[k] = static_cast<uint8_t>(base64_chars.find(char_array_4[k]));
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (int k = 0; k < i - 1; k++) {
            result.push_back(char_array_3[k]);
        }
    }

    return result;
}
ZmqMsgHandle::ZmqMsgHandle(std::string netConfigPath, int ServerOrClient)
{
    InitEnv(netConfigPath, ServerOrClient);
    ZMQ_ERR = 0;
}
ZmqMsgHandle::~ZmqMsgHandle()
{
    delete m_socketHandle;
}
void ZmqMsgHandle::InitEnv(std::string netConfigPath, int ServerOrClient)
{
    std::cout << "ZMQ init....."<< std::endl;
    if( nullptr == m_socketHandle)
    {
        m_ServerOrClient = ServerOrClient;
        m_socketHandle = new SocketHandle();
        m_socketHandle->InitSocket(netConfigPath, ServerOrClient);
    }
}


void ZmqMsgHandle::sendTaskInstructions(std::string netto,std::string task, cv::Mat srcImg) {
    m_sockName1 = netto;
    std::vector<std::vector<uchar>>  MLProcessData; 
    if (srcImg.channels() == 1) {//单通道转换成三通道
        cv::cvtColor(srcImg, srcImg, cv::COLOR_GRAY2BGR);
    }
    std::vector<uchar> myTaskName;
    for (char c : task) {
        uchar u = static_cast<uchar>(c);
        myTaskName.push_back(u);
    }
    //图片直接编码成二进制发送
    std::vector<uchar> imageBuffer;
    cv::imencode(".bmp", srcImg, imageBuffer);
    MLProcessData.push_back(myTaskName);
    MLProcessData.push_back(imageBuffer);
    m_TaskData.emplace_back(MLProcessData);
}
void ZmqMsgHandle::sendTaskInstructions(std::string netto, Zmqpkg::ZMQ_FronterReciveDataTypeA m_waitSendInfo) {
    m_sockName2 = netto;
    Fronter_SendResult.push_back(m_waitSendInfo);
}
void ZmqMsgHandle::sendTaskInstructions(std::string netto, nlohmann::json js){
    m_sockName3 = netto;
    m_AgrCurCFG.emplace_back(js);
}

void ZmqMsgHandle::run() {
    switch (m_ServerOrClient)
    {
    case 0:
        ZmqMsgHandle::ThreadSvrRun();
        break;
    case 1:
        ZmqMsgHandle::ThreadSendRun();
        break;
    case 2:
        ZmqMsgHandle::ThreadSvrRun();
        ZmqMsgHandle::ThreadSendRun();
        break;
    }
}
void ZmqMsgHandle::ThreadSvrRun() {
    auto selfRevServerTask = [&]() {ZmqMsgHandle::SvrRun(); };
    std::thread  selfRevServerTh(selfRevServerTask);
    selfRevServerTh.detach();//守护接受服务端线程
    std::cout << "Daemon Listening Server Thread ....." << std::endl;
}
void ZmqMsgHandle::ThreadSendRun() {
    auto selfSendServerTask = [&]() {ZmqMsgHandle::SendRun(); };
    std::thread selfSendServerTh(selfSendServerTask);
    selfSendServerTh.detach();//守护发送服务端线程
    std::cout << "Daemon Listening Server Thread ....." << std::endl;
}
void ZmqMsgHandle::SvrRun() {
    std::cout << "Recieve server run ..." << std::endl;
    std::vector<zmq::pollitem_t> items = {
            { *m_socketHandle->GetSocket("cell_in_func")->socket(), 0, ZMQ_POLLIN, 0 },
            { *m_socketHandle->GetSocket("cell_in_agri")->socket(), 0, ZMQ_POLLIN, 0 }
    };
    int i = 0;
    while (true)
    {
        zmq::poll(items.data(), items.size(), std::chrono::milliseconds{ 1000 });
        Zmqpkg::ZMessageArrayu rev_Arrayu;
        Zmqpkg::ZMessageList_t rev_msgUnpacked;
        if (items[0].revents & ZMQ_POLLIN)
        {
            Zmqpkg::ZMessage zmsg;
            m_socketHandle->GetMessage_zmq("cell_in_func", zmsg, rev_msgUnpacked);
            zmsg.clear();
            RevResultData_MsgPack.push_back(rev_msgUnpacked.front());
            rev_msgUnpacked.pop_front();

        }
        if (items[1].revents & ZMQ_POLLIN)
        {
            Zmqpkg::ZMessage zmsg;
            Zmqpkg::ZMessageArray marry;
            Zmqpkg::ZSocket* socket = m_socketHandle->GetSocket("cell_in_agri");
            socket->getMessage(zmsg);
            zmsg.getMessage(marry);

            std::string str(marry[0].data(), marry[0].size());
            nlohmann::json js = nlohmann::json::parse(str);
            std::cout << " mode: " << js["Mode"] << " task: " << js["Task"] << std::endl;
            RevResultData_Json.push_back(js);
            
        }
    }
}
//void ZmqMsgHandle::SendRun(){
//    std::cout<<"Send server run ..."<< std::endl;
//    int heartbeat_interval = 2;
//    nlohmann::json sendHeartBeatJson;
//    sendHeartBeatJson["heartbeatinfo"] = "HEARTBEAT";
//    nlohmann::json revHeartBeatJson;
//    while(true)
//    {
//        m_socketHandle->SendMessage_zmq(CELL_MLA_AGRITHM, sendHeartBeatJson);
//        m_socketHandle->GetMessage_zmq(CELL_MLA_AGRITHM, revHeartBeatJson);
//        if (revHeartBeatJson["heartbeatinfoACK"] == "HEARTBEAT_ACK") {
//            std::cout << "Received heartbeat acknowledgment" << std::endl;
//        }
//        if (!m_AgrCurCFG.empty())
//        {
//            nlohmann::json revJs;
//            if (m_socketHandle->SendMessage_zmq(CELL_MLA_AGRITHM, m_AgrCurCFG.front()))
//            {
//                m_AgrCurCFG.pop_front();
//                m_socketHandle->GetMessage_zmq(CELL_MLA_AGRITHM, revJs);
//                std::cout << __func__ << ": " << "ZMQ ,MLA Sending message to CELL_Agrithm successful ..." << std::endl;
//                std::cout << revJs.dump().c_str() << std::endl;
//            }
//        }
//        // 等待一段时间再发送下一个心跳
//        std::this_thread::sleep_for(std::chrono::seconds(heartbeat_interval));
//    }
//}
void ZmqMsgHandle::SendRun(){ 
    std::cout<<"Send Client run ..."<< std::endl;
    while(true)
    {
        if (!m_TaskData.empty())
        {
            Zmqpkg::ZMessageArrayu rev_Arrayu;
            auto start1 = std::chrono::system_clock::now();
            if (m_socketHandle->SendMessage_vvuchar(m_sockName1, m_TaskData.front()))
            {
                m_TaskData.pop_front();
                std::cout << __func__ << ": " << "ZMQ  Sending message successfully ..." << std::endl;
                auto end1 = std::chrono::system_clock::now();
                auto tc1 = (double)std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count() / 1000.;
                std::cout << "zmq time: " << tc1 << " ms" << std::endl;
            }
        }
        if (!Fronter_SendResult.empty())
        {
            Zmqpkg::ZMessageArrayu rev_Arrayu;
            auto start1 = std::chrono::system_clock::now();
            if (m_socketHandle->SendMessage_MsgPack(m_sockName2, Fronter_SendResult.front()))
            {
                Fronter_SendResult.pop_front();
                std::cout << __func__ << ": " << "ZMQ  Sending message successfully ..." << std::endl;
                auto end1 = std::chrono::system_clock::now();
                auto tc1 = (double)std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count() / 1000.;
                std::cout << "zmq time: " << tc1 << " ms" << std::endl;
            }
        }
        if (!m_AgrCurCFG.empty())
        {
            auto start1 = std::chrono::system_clock::now();
            if (m_socketHandle->SendMessage_zmq(m_sockName3, m_AgrCurCFG.front()))
            {
                std::cout << __func__ << ": " << "ZMQ  Sending message successfully ..." << std::endl;
                auto end1 = std::chrono::system_clock::now();
                auto tc1 = (double)std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1).count() / 1000.;
                std::cout << "zmq time: " << tc1 << " ms" << std::endl;
            }
        }
    }
}
