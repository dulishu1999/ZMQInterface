//
// Created by fp on 2021/11/12.
//
#ifndef ZMQ_TEST_ZMQ_SOCKET_H
#define ZMQ_TEST_ZMQ_SOCKET_H
#include "json.hpp"
#include "zmq/zmq_msg.h"
#include"zmq.hpp"
#include"zmq_addon.hpp"
#include <map>
#include <string>
#include <vector>
#include "zmq/zmq_msg.h"

namespace  Zmqpkg
{
    class  ZSocket
    {
    public:
        ZSocket(zmq::socket_type type);
        ZSocket(zmq::context_t *contex,zmq::socket_type type);
        void initSocket();
        bool sendMessage(ZMessage & msg);
        bool getMessage(ZMessage & msg);
        zmq::socket_t  * socket() ;

        /*!
         *
         * @param addr
         */
        void bind(std::string const &addr) { bind(addr.c_str()); }
        void bind(const char *addr_)
        {
            m_socket->bind(addr_);
        }
        void unbind(std::string const &addr) { unbind(addr.c_str()); }
        void unbind(const char *addr_)
        {
            m_socket->unbind(addr_);
        }
        void connect(std::string const &addr) { connect(addr.c_str()); }
        void connect(const char *addr_)
        {
            m_socket->connect(addr_);
        }

        void disconnect(std::string const &addr) { disconnect(addr.c_str()); }
        void disconnect(const char *addr_)
        {
            m_socket->disconnect(addr_);
        }
    protected:
        zmq::context_t * m_context = nullptr;
        zmq::socket_t  * m_socket = nullptr;
        zmq::socket_type m_socktype;
    };
    static zmq::context_t *gs_context = new zmq::context_t(4);

    bool  sendMessage(ZSocket* socket, const ZMessageArray &vecMsg);
    bool  sendMessage(ZSocket* socket, const nlohmann::json & js);

    bool  getMessage(ZSocket* socket, ZMessageArray& vecMsg);
    bool  getMessage(ZSocket* socket, nlohmann::json & js);
}

#endif  // ZMQ_TEST_ZMQ_SOCKET_H
