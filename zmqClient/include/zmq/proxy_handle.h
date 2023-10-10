//
// Created by stan on 2021/10/27.
//

#ifndef ALL_PROXY_HANDLE_H
#define ALL_PROXY_HANDLE_H
#include "json.hpp"
#include "zmq/common_def.h"
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "zmq/toml_config.h"
#include <stdlib.h>

class  InProxyHandles
{
public:
    InProxyHandles();
    virtual ~InProxyHandles();
    void HeaderInfo( nlohmann::json &js );
    void ProcessTypeArg(nlohmann::json &js);
    void SelectCamera( std::string& cameraName, std::string& cameraId );

    void Clear();
    std::string MakeKey();
public:
    ECmd getMCmd() const;
    ETaskType getMTask() const;

public:
    std::string m_moudleName;
    ECmd m_cmd = E_CMD_UNKNOWN;  //
    ETaskType m_task = E_TASK_TYPE_UNKNOWN; //
    int m_erro = 0;
};

class  OutProxyHandles
{
public:
    OutProxyHandles();
    void ProcessOutParam(nlohmann::json &js );
};


#endif //ALL_PROXY_HANDLE_H
