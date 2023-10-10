#include "zmq/proxy_handle.h"


InProxyHandles::~InProxyHandles()
{
}

InProxyHandles::InProxyHandles()
{

}

ECmd InProxyHandles::getMCmd() const
{
    return m_cmd;
}

ETaskType InProxyHandles::getMTask() const
{
    return m_task;
}
void InProxyHandles::ProcessTypeArg(nlohmann::json &js)
{
    // 配置相机信息
    std::string  cameraName;
    std::string  cameraId;
    SelectCamera(cameraName, cameraName);
}

void InProxyHandles::SelectCamera( std::string& cameraName, std::string& cameraId )
{
}

void InProxyHandles::HeaderInfo( nlohmann::json& js )
{
    Clear();
    const char * cmd_key = MSG_CMD;
    if( js.contains(cmd_key))
    {
        m_cmd = js[cmd_key];
    }
    cmd_key = MSG_TASK_TYPE;
    if( js.contains(cmd_key))
    {
        m_task = js[cmd_key];
    }
    cmd_key = MSG_CELL_NAME;
    if( js.contains(cmd_key))
    {
        m_moudleName = js[cmd_key];
    }
}
void InProxyHandles::Clear()
{
    m_cmd = E_CMD_UNKNOWN;
    m_task = E_TASK_TYPE_UNKNOWN;
    m_moudleName.clear();
}
std::string InProxyHandles::MakeKey()
{
    return std::to_string(m_cmd)+ "_" + std::to_string(m_task);
}

OutProxyHandles::OutProxyHandles() {}

void OutProxyHandles::ProcessOutParam( nlohmann::json& js )
{
    js[MSG_CELL_NAME] = TomlConfig::Instance().m_cellName;
}

