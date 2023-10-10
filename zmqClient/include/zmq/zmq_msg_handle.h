/*!
 * 本程序协议 待定
 */

#ifndef _ZMQ_MSG_HANDLE_H_
#define _ZMQ_MSG_HANDLE_H_
#include "zmq/socket_handle.h"

#include "zmq/toml_config.h"

#include "zmq/proxy_handle.h"

#include "zmq/common_def.h"
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include "json.hpp"
#include "opencv2/core.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <Eigen/Geometry>
#include <Eigen/Dense>
#include <cstdlib>
#include <iostream>
#include <list>
#include "timer.h"
#include <mutex>
#include <map>
#include <cstdlib>
#include <string>
#include <fstream>
using namespace std;
struct  ZMQ_ReciveDataType
{
    std::string Netto;
    std::string Mode;
    std::string Task;
};

class  ZmqMsgHandle
{
public:
    ZmqMsgHandle(std::string netConfigPath, int ServerOrClient);
    ~ZmqMsgHandle();
    /*!
    * 服务运行
    */ 
    void run();
    void sendTaskInstructions(std::string netto,std::string task,cv::Mat srcImg);
    void sendTaskInstructions(std::string netto, Zmqpkg::ZMQ_FronterReciveDataTypeA m_waitSendInfo);
    void sendTaskInstructions(std::string netto, nlohmann::json js);

protected:
    /*!
    *初始化环境，从toml读取配置 初始化服务
    */
    void InitEnv(std::string netConfigPath, int ServerOrClient);

private:
    void ThreadSvrRun();
    void ThreadSendRun();
    void SvrRun();
    void SendRun();
        
    std::list<nlohmann::json>  m_AgrCurCFG;
    std::mutex my_mutex_1;
    std::string m_sockName1;
    std::string m_sockName2;
    std::string m_sockName3;
    int m_ServerOrClient;
    std::list<std::vector<std::vector<uchar>>>  m_TaskData;//
    std::list<ZMQ_ReciveDataType> Agri_waitDataList;
    std::list<ZMQ_ReciveDataType> Func_waitDataList;
    std::list<ZMQ_ReciveDataType> Func_RevDataList;
    std::list<Zmqpkg::ZMQ_FronterReciveDataTypeA> Fronter_SendResult;


    //接受数据缓存LIST  取数据也要从这里面取,切记每次取出一个list成员要将该成员pop掉
    std::list<nlohmann::json> RevResultData_Json;
    std::list<Zmqpkg::ZMQ_FronterReciveDataTypeA> RevResultData_MsgPack;

public:
    int ZMQ_ERR;

    InProxyHandles m_inHandle;
    OutProxyHandles m_outHandle;
    SocketHandle *m_socketHandle;
};
#endif //ALL_ZMQ_MSG_HANDLE_H
