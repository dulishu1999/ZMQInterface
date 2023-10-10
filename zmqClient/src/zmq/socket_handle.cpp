#include "zmq/zmq_msg_handle.h"
#include "zmq/socket_handle.h"

//#include "utils/timer.h"

SocketHandle::SocketHandle() {}
SocketHandle::~SocketHandle() {}
void SocketHandle::InitSocket(std::string netConfigPath, int ServerOrClient)
{
    std::cout <<"netConfigPath: " << netConfigPath << std::endl;
    TomlConfig::Instance().ReadConfig(netConfigPath, ServerOrClient);
    m_context = new zmq::context_t(2);
    for (auto item : TomlConfig::Instance().m_InEpMap)//服务端绑定
    {
        m_socket[item.first] = new Zmqpkg::ZSocket(m_context, zmq::socket_type::router);
        m_socket[item.first]->bind(item.second);
        std::cout << item.first << " bind " << item.second << "\n";
    }
    for (auto item : TomlConfig::Instance().m_outEpMap)//客户端连接
    {
        m_socket[item.first] = new Zmqpkg::ZSocket(m_context, zmq::socket_type::dealer);    
        m_socket[item.first]->socket()->set(zmq::sockopt::reconnect_ivl, 100);//ms
        m_socket[item.first]->socket()->set(zmq::sockopt::routing_id, item.first);
        while (true) {
            try {
                m_socket[item.first]->connect(item.second);
                break; // 连接成功，退出循环
            }
            catch (const zmq::error_t& e) {
                // 连接失败，可以在此处添加日志或其他处理
                std::cerr << item.first << " failed to connect: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1)); // 休眠一段时间后重试
            }
        }
        std::cout << item.first << " connect " << item.second << std::endl;
    }


}

bool SocketHandle::GetMessage_zmq( const std::string& module, nlohmann::json& js )
{
    Zmqpkg::ZSocket* socket = GetSocket(module);
    if(!socket)
    {
        return false;
    }
    Zmqpkg::ZMessage message;
    socket->getMessage(message);
    Zmqpkg::ZMessageArray marry;
    message.getMessage(marry);

    std::string str ((char *)marry[0].data(), marry[0].size());
    try
    {
        js = nlohmann::json::parse(str);
    }
    catch (nlohmann::json::parse_error& e)
    {
        return  false;
    }
    return str.size() > 0;
}
bool SocketHandle::SendMessage_zmq( const std::string& module, const nlohmann::json& js )
{
    //std::cout << __func__ << module << " send0 : \n";
    Zmqpkg::ZSocket* socket = GetSocket(module);
    if(!socket)
    {
        std::cout << __func__ << module << " no sockect : \n";
        return false;
    }
    //std::cout << __func__ << module << " send1 : \n";
    Zmqpkg::ZMessage msg;
    msg.addMessage(js.dump().c_str());
    return  socket->sendMessage(msg);
}
bool SocketHandle::SendMessage_zmq(const std::string& module, Zmqpkg::ZMessage& msg, const nlohmann::json& js)
{
    //std::cout << __func__ << module << " send0 : \n";
    Zmqpkg::ZSocket* socket = GetSocket(module);
    if (!socket)
    {
        std::cout << __func__ << module << " no sockect : \n";
        return false;
    }
    //std::cout << __func__ << module << " send1 : \n";
    msg.addMessage(js.dump().c_str());
    return  socket->sendMessage(msg);
}
bool SocketHandle::SendMessage_vvuchar(const std::string& module, Zmqpkg::ZMessage& msg,Zmqpkg::ZMessageArrayu& mMap)
{
    Zmqpkg::ZSocket* socket = GetSocket(module);
    if (!socket)
    {
        std::cout << __func__ << module << " no sockect : \n";
        return false;
    }
    msg.setMessage(mMap);
    return  socket->sendMessage(msg);
}
bool SocketHandle::SendMessage_vvuchar(const std::string& module, Zmqpkg::ZMessageArrayu& mMap)
{
    Zmqpkg::ZSocket* socket = GetSocket(module);
    if (!socket)
    {
        std::cout << __func__ << module << " no sockect : \n";
        return false;
    }
    m_msg.setMessage(mMap);
    return  socket->sendMessage(m_msg);
}
bool SocketHandle::SendMessage_MsgPack(const std::string& module, Zmqpkg::ZMQ_FronterReciveDataTypeA& m_A) {
    Zmqpkg::ZSocket* socket = GetSocket(module);
    if (!socket)
    {
        std::cout << __func__ << module << " no sockect : \n";
        return false;
    }
    Zmqpkg::ZMQ_FronterReciveDataTypeB m_B;
    m_B.camId = m_A.camId;
    m_B.serverId = m_A.serverId;
    m_B.NgOrOk = m_A.NgOrOk;
    cv::imencode(".bmp", m_A.srcImage, m_B.srcImage_uchar);
    for (auto m_rect : m_A.BoxRect) {
        std::vector<float> rect_array;
        rect_array.push_back(m_rect.x);
        rect_array.push_back(m_rect.y);
        rect_array.push_back(m_rect.width);
        rect_array.push_back(m_rect.height);
        m_B.BoxRect.push_back(rect_array);
    }
    // 使用 MessagePack 序列化数据
    std::stringstream sbuf;
    msgpack::pack(sbuf, m_B);
    zmq::message_t m_ssage(sbuf.str().data(), sbuf.str().size());

    m_msg.setMessage(m_ssage);
    return  socket->sendMessage(m_msg);

}
Zmqpkg::ZSocket* SocketHandle::GetSocket( const std::string& module )
{
    if(m_socket.find(module) == m_socket.end())
    {
        return nullptr;
    }
    return m_socket[module];
}
bool SocketHandle::GetMessage_zmq(const string& module, Zmqpkg::ZMessage &msg, Zmqpkg::ZMessageArrayu& m_arry)
{
    Zmqpkg::ZSocket* socket = GetSocket(module);
    if (!socket)
    {
        std::cout << __func__ << module << " no sockect : \n";
        return false;
    }
    socket->getMessage(msg);
    return  msg.getMessage(m_arry);
}
bool SocketHandle::GetMessage_zmq(const string& module, Zmqpkg::ZMessage& msg, Zmqpkg::ZMessageList_t& m_arry)
{
    Zmqpkg::ZSocket* socket = GetSocket(module);
    if (!socket)
    {
        std::cout << __func__ << module << " no sockect : \n";
        return false;
    }
    socket->getMessage(msg);
    return  msg.getMessage(m_arry);
}
bool SocketHandle::GetMessage_zmq(const string &module, Zmqpkg::ZMessageArray &m_arry)
{
    //std::cout << __func__ << module << " send0 : \n";
    Zmqpkg::ZSocket* socket = GetSocket(module);
    if(!socket)
    {
        std::cout << __func__ << module << " no sockect : \n";
        return false;
    }
    //std::cout << __func__ << module << " send1 : \n";
    Zmqpkg::ZMessage msg;
    socket->getMessage(msg);
    return  msg.getMessage(m_arry);
}
bool SocketHandle::GetMessage_zmq(const std::string& module, Zmqpkg::ZMessage & zmsg)
{
    Zmqpkg::ZSocket* socket = GetSocket(module);
    if(!socket)
    {
        std::cout << __func__ << module << " no sockect : \n";
        return false;
    }
   return socket->getMessage(zmsg);
}