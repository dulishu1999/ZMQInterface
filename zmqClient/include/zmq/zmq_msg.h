//
// Created by fp on 2021/11/12.
//

#ifndef ZMQ_TEST_ZMQ_MSG_H
#define ZMQ_TEST_ZMQ_MSG_H
#include "json.hpp"
#include <vector>
#include <list>
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include <iostream>
#include "opencv2/core.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <msgpack.hpp>

namespace Zmqpkg
{

struct  ZMQ_FronterReciveDataTypeA
{
    std::string serverId;
    std::string camId;
    std::string NgOrOk;
    std::vector<cv::Rect> BoxRect;
    cv::Mat srcImage;

};
struct  ZMQ_FronterReciveDataTypeB
{
    std::string serverId;
    std::string camId;
    std::string NgOrOk;
    std::vector<std::vector<float>> BoxRect;
    std::vector<unsigned char> srcImage_uchar;
    MSGPACK_DEFINE(serverId, camId, NgOrOk, BoxRect, srcImage_uchar);

};
using zchar=char;
using zuchar= unsigned  char;
using ZMessageItem = std::vector<zchar> ;
using ZMessageItemu = std::vector<zuchar> ;
using ZMessageArray = std::vector< ZMessageItem >;
using ZMessageArrayu = std::vector< ZMessageItemu>;
using ZMessageList = std::list< ZMessageItem >  ;
using ZMessageList_t = std::list<Zmqpkg::ZMQ_FronterReciveDataTypeA>;

class ZMessage
{
public:
    ZMessage();
    ~ZMessage();
    bool getMessage(ZMessageArray& vecmsg);
    bool getMessage(ZMessageArrayu& vecmsg);
    bool getMessage(ZMessageItem& msg, int idx);
    bool getMessage(ZMessageList_t& vecmsg);
    bool addMessage(ZMessageItem& msg);
    bool addMessage(ZMessageItemu& msg);
    bool addMessage( zchar* data, size_t num);
    bool setMessage(const ZMessageArray& vecmsg );
    bool setMessage(const ZMessageArrayu& vecmsg);
    bool setMessage(zmq::message_t& zmsg);
    bool addMessage(const std::string& str);
    size_t size();
    void print();public:
    void clear();
public:
    const std::string& getMId() const;
    void               setMId( const std::string& mId );
    std::list< zmq::message_t >& getMVecmsg();
    void setMVecmsg(const ZMessageArray & vecmsg);
    zmq::message_t * getZMessage(int idx);
protected:
    std::list< zmq::message_t> m_msglist;
    std::string m_id;
};
bool vec2json(const ZMessageItem& msg, nlohmann::json& js);
bool json2vec(const nlohmann::json& js, ZMessageItem& msg);
bool getMessage(ZMessage &msg, int idx, nlohmann::json& js);
void setMVecmsg(ZMessage &msg, const nlohmann::json& js);
}

#endif  // ZMQ_TEST_ZMQ_MSG_H
