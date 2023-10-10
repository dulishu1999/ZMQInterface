#ifndef _TOML_CONFIG_H_
#define _TOML_CONFIG_H_
#include <string>
#include <map>
#include "zmq.hpp"
#include "zmq_addon.hpp"
#include <iostream>
#include <string>
#include "cpptoml.h"
class  TomlConfig
{
public:
    static TomlConfig & Instance();
    void ReadConfig(const std::string & tomlpath, int ServerOrClient);
    void x_ReadConfig(const std::string& tomlpath);
    void Reload();

protected:
    TomlConfig();
    std::map<std::string, std::string> readNetIn(const cpptoml::table& netInTable);
    std::map<std::string, std::string> readNetTo(const cpptoml::table& netToTable);
private:
    /*!
     * 配置文件名称
     */
    std::string m_file;
    /*!
     * 节点名称
     */
public:
    std::string m_cellName;
    /*!
     * 节点id
     */
    std::string m_cellId;
    /*!
     * 本机消息入口
     */
    /*!
     * 处理结果输出 端点
     */
    std::map<std::string, std::string> m_outEpMap;
    std::map<std::string, std::string> m_InEpMap;
};

#endif // ROBOTCONFIG_H
