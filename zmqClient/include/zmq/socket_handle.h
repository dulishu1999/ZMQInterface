
#ifndef _SOCKET_HANDLE_H
#define _SOCKET_HANDLE_H
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "json.hpp"
#include "zmq/zmq_socket.h"
#include "zmq/toml_config.h"
#include <map>
#include <string>
#include <experimental/filesystem>
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <msgpack.hpp>


class  SocketHandle
{
public:
    SocketHandle();
    virtual ~SocketHandle();
    void InitSocket(std::string netConfigPath, int ServerOrClient);

    bool GetMessage_zmq(const std::string& module,nlohmann::json &js);
    bool SendMessage_zmq(const std::string& module, const nlohmann::json& js);
    bool SendMessage_zmq(const std::string& module, Zmqpkg::ZMessage& msg, const nlohmann::json& js);
    bool SendMessage_vvuchar(const std::string& module, Zmqpkg::ZMessageArrayu& mMap);
    bool SendMessage_vvuchar(const std::string& module, Zmqpkg::ZMessage& msg, Zmqpkg::ZMessageArrayu& mMap);
    bool SendMessage_MsgPack(const std::string& module, Zmqpkg::ZMQ_FronterReciveDataTypeA& m_A);
    bool GetMessage_zmq(const string& module, Zmqpkg::ZMessage& msg, Zmqpkg::ZMessageList_t& m_arry);

    bool GetMessage_zmq(const std::string& module, Zmqpkg::ZMessage& msg, Zmqpkg::ZMessageArrayu& m_arry);
    bool GetMessage_zmq(const std::string& module, Zmqpkg::ZMessageArray & m_arry);
    bool GetMessage_zmq(const std::string& module, Zmqpkg::ZMessage & zmsg);

    Zmqpkg::ZSocket* GetSocket(const std::string& module);
public:
    zmq::context_t *m_context = nullptr;
    /*!
     * socket 入口  处理支持的协议
     */
    std::map<std::string, Zmqpkg::ZSocket *>m_socket;
private:
    Zmqpkg::ZMessage m_msg;
};

#endif  // ALL_SOCKET_HANDLE_H
