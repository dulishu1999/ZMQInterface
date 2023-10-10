#include "zmq/zmq_msg.h"

namespace Zmqpkg
{
ZMessage::ZMessage() {}
ZMessage::~ZMessage() {}
bool ZMessage::setMessage(const ZMessageArray& vecmsg )
{
    int count = vecmsg.size();
    if( 0 == count)
    {
        m_msglist.clear();
        return true;
    }
    for(int i = 0; i < count; ++i)
    {
        const std::vector< zchar > & msg = vecmsg[i];
        zmq::message_t zmsg(msg.size());
        memcpy(zmsg.data(), msg.data(), msg.size());
        m_msglist.push_back(std::move(zmsg));
    }
    return true;
}
bool ZMessage::setMessage(const ZMessageArrayu& vecmsg)
{
    int count = vecmsg.size();
    if (0 == count)
    {
        m_msglist.clear();
        return true;
    }
    if (!m_msglist.empty()) {
        m_msglist.clear();
    }
    for (int i = 0; i < count; ++i)
    {
        const std::vector<zuchar>& msg = vecmsg[i];
        zmq::message_t zmsg(msg.size());
        memcpy(zmsg.data(), msg.data(), msg.size());
        m_msglist.push_back(std::move(zmsg));
    }
    return true;
}
bool ZMessage::setMessage(zmq::message_t& zmsg)
{
    m_msglist.push_back(std::move(zmsg));
    return true;
}
bool ZMessage::getMessage( ZMessageItem& msg, int idx )
{
    if(idx >= size())
    {
        return false;
    }
    zmq::message_t * pzmsg_t = getZMessage(idx);
    if( nullptr == pzmsg_t)
    {
        return false;
    }
    size_t sz = pzmsg_t->size();
    msg.resize(sz,0);
    memcpy(msg.data(), pzmsg_t->data(), sz);
    sz = msg.size();
    return true;
}
bool ZMessage::getMessage(ZMessageArrayu& vecmsg)
{
    int count = m_msglist.size();
    if (0 == count)
    {
        return false;
    }
    vecmsg.resize(count);
    auto it = m_msglist.begin();
    for (int i = 0; i < count; ++i)
    {
        auto& item = *it;
        if (item.size() > 0)
        {
            vecmsg[i].resize(item.size());
            memcpy(vecmsg[i].data(), item.data(), item.size());
        }
        ++it;
    }
    return true;
}
bool ZMessage::getMessage(Zmqpkg::ZMessageList_t& vecmsg)
{
    int count = m_msglist.size();
    if (0 == count)
    {
        return false;
    }

    auto it = m_msglist.begin();
    for (int i = 0; i < count; ++i)
    {
        auto& item = *it;
        ZMQ_FronterReciveDataTypeA m_a;
        if (item.size() > 0)
        {
            // 反序列化数据为 ZMQ_FronterReciveDataType 对象
            msgpack::object_handle oh = msgpack::unpack(static_cast<const char*>(item.data()), item.size());
            msgpack::object obj = oh.get();
            ZMQ_FronterReciveDataTypeB receivedData;
            obj.convert(receivedData);
            m_a.camId = receivedData.camId;
            m_a.serverId = receivedData.serverId;
            m_a.NgOrOk = receivedData.NgOrOk;
            m_a.srcImage = cv::imdecode(receivedData.srcImage_uchar, cv::IMREAD_COLOR);
            for (auto box : receivedData.BoxRect) {
                m_a.BoxRect.push_back(cv::Rect(box[0], box[1], box[2], box[3]));
            }
            vecmsg.push_back(m_a);

        }
        ++it;
    }
    return true;
}

bool ZMessage::getMessage(ZMessageArray & vecmsg)
{
    int count = m_msglist.size();
    if( 0 == count)
    {
        return false;
    }
    vecmsg.resize(count);
    auto it = m_msglist.begin();
    for(int i = 0; i < count; ++i)
    {
        auto & item  = *it;
        if( item.size() > 0)
        {
            vecmsg[i].resize( item.size());
            memcpy(vecmsg[i].data(),item.data(), item.size());
        }
        ++it;
    }
    return true;
}
size_t ZMessage::size()
{
    return m_msglist.size();
}
std::list< zmq::message_t >& ZMessage::getMVecmsg()
{
    return m_msglist;
}
void ZMessage::setMVecmsg(const ZMessageArray & vecmsg )
{
    size_t count = vecmsg.size();
    m_msglist.resize(count);
    for(int i = 0; i < count; ++i)
    {
        zmq::message_t msg;
        msg.rebuild(vecmsg[i].size());
        memcpy(msg.data(), msg.data(), vecmsg[i].size());
        m_msglist.push_back(std::move(msg));
    }
}
void ZMessage::print()
{
    size_t count = size();
    int i = 0;
    auto it = m_msglist.begin();
    while (i++ < count)
    {
        auto &item = *it;
        std::string str((char*)item.data(), item.size());
        std::cout <<str << "\n";
        ++it;
    }
    std::cout << "----------------------------------------------\n";
}
const std::string& ZMessage::getMId() const
{
    return m_id;
}
void ZMessage::setMId( const std::string& mId )
{
    m_id = mId;
}

void ZMessage::clear()
{
    m_id.clear();
    m_msglist.clear();
}

bool ZMessage::addMessage( ZMessageItem& msg )
{
    return addMessage((zchar* )msg.data(), msg.size());
}

bool ZMessage::addMessage( ZMessageItemu& msg )
{
    return addMessage((zchar* )msg.data(), msg.size());
}

bool ZMessage::addMessage( const std::string& str )
{
    return addMessage((zchar* )str.c_str(), str.size());
}

bool ZMessage::addMessage( zchar* data, size_t sz )
{
    zmq::message_t msg;
    msg.rebuild(sz);
    memcpy(msg.data(), data, sz);
    m_msglist.push_back(std::move(msg));
    return true;
}
zmq::message_t* ZMessage::getZMessage( int idx )
{
    zmq::message_t * pzmsg_t = nullptr;
    auto it = m_msglist.begin();
    for(int i = 0; i < m_msglist.size(); ++i)
    {
        if(i == idx)
        {
            pzmsg_t = &(*it);
            break;
        }
        ++it;
    }
    return pzmsg_t;
}


bool vec2json(const ZMessageItem& msg, nlohmann::json& js)
{
    js = nlohmann::json::parse(msg.data());
    return true;
}

bool json2vec( const nlohmann::json& js, ZMessageItem & msg )
{
    size_t count = js.dump().size();
    msg.resize(count);
    memcpy(msg.data(), js.dump().c_str(),count);
    return true;
}

bool getMessage( ZMessage& msg, int idx, nlohmann::json& js )
{
    std::vector< zchar> buff;
    if(msg.getMessage(buff, idx))
    {
        return  vec2json(buff, js);
    }
    return false;
}
void setMVecmsg( ZMessage& msg, const nlohmann::json& js )
{
    std::vector<zchar > buff;
    Zmqpkg::json2vec(js,buff);
    std::vector< std::vector< zchar > > vecmsg {buff};
    msg.setMVecmsg(vecmsg);
}
}