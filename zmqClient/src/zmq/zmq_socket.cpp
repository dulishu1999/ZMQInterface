//
// Created by fp on 2021/11/12.
//

#include "zmq/zmq_socket.h"
namespace  Zmqpkg
{
ZSocket::ZSocket( zmq::context_t* contex, zmq::socket_type type ) : m_context( contex ),m_socktype(type)
{
    initSocket();
}
ZSocket::ZSocket( zmq::socket_type type ) : m_context( gs_context ),m_socktype(type)
{
    initSocket();
}
bool ZSocket::sendMessage( ZMessage& msg )
{
    if(m_socktype == zmq::socket_type::router)
    {
        zmq::message_t zid(msg.getMId().size());
        memcpy(zid.data(), msg.getMId().data(), msg.getMId().size());
        std::cout<<"ROUTER ZID: " << msg.getMId() << std::endl;
        m_socket->send(zid,zmq::send_flags::sndmore|zmq::send_flags::dontwait);
    }
    zmq::send_result_t rst = zmq::send_multipart( *m_socket, msg.getMVecmsg(), zmq::send_flags::dontwait );
    return rst.has_value() && rst.value() > 0;
}
bool ZSocket::getMessage( ZMessage& msg )
{
    auto & vecbuf =  msg.getMVecmsg();
    if (!vecbuf.empty()) {
        vecbuf.clear();
    }
    zmq::recv_result_t rst = zmq::recv_multipart( *m_socket, std::back_inserter(vecbuf), zmq::recv_flags::none );
    if(vecbuf.size() > 1 && m_socktype == zmq::socket_type::router)
    {
        auto &item = *vecbuf.begin();
        std::string id ((char *)item.data(), item.size()) ;
        vecbuf.erase(vecbuf.begin());
        msg.setMId(id);
        std::cout<< "m_id: " << msg.getMId() << std::endl;
    }
    return rst.has_value() && rst.value() > 0;
}

zmq::socket_t* ZSocket::socket()
{
    return m_socket;
}
void ZSocket::initSocket( )
{
    m_socket = new zmq::socket_t( *m_context, m_socktype );
}

bool sendMessage( ZSocket* socket, const ZMessageArray & vecMsg )
{
    ZMessage msg;
    msg.setMVecmsg( vecMsg );
    return socket->sendMessage( msg );
}
bool sendMessage( ZSocket* socket, const nlohmann::json& js )
{
    ZMessage msg;
    msg.setMVecmsg( js );
    return socket->sendMessage( msg );
}
bool getMessage( ZSocket* socket, ZMessageArray& vecMsg )
{
    ZMessage msg;
    if ( !socket->getMessage( msg ) )
    {
        return false;
    }
    return msg.getMessage( vecMsg );
}
bool getMessage( ZSocket* socket, nlohmann::json& js )
{
    ZMessage msg;
    if ( !socket->getMessage( msg ) )
    {
        return false;
    }
    ZMessageItem buff;
    if ( !msg.getMessage( buff, 0 ) )
    {
        return false;
    }
    return vec2json( buff, js );
}
}