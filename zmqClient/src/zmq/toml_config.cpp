#include "zmq/toml_config.h"
#include "zmq/zmq_msg_handle.h"

TomlConfig &TomlConfig::Instance()
{
    static TomlConfig inst;
    return inst;
}

TomlConfig::TomlConfig()
{
    
}

std::map<std::string, std::string> TomlConfig::readNetIn(const cpptoml::table& netInTable) {
    std::map<std::string, std::string> InEpMap;
  
    const auto agrithnmTable = netInTable.get_table("agrithnm");
    if (agrithnmTable) {
        const auto name = agrithnmTable->get_as<std::string>("name");
        const auto endPoint = agrithnmTable->get_as<std::string>("end_point");
        InEpMap[name->c_str()] = endPoint->c_str();
        if (name && endPoint) {
            std::cout << "NetIn " << ": " << *name << ", End Point: " << *endPoint << std::endl;
        }
    }
    const auto functionTable = netInTable.get_table("function");
    if (functionTable) {
        const auto name = functionTable->get_as<std::string>("name");
        const auto endPoint = functionTable->get_as<std::string>("end_point");
        InEpMap[name->c_str()] = endPoint->c_str();
        if (name && endPoint) {
            std::cout << "NetIn " << ": " << *name << ", End Point: " << *endPoint << std::endl;
        }
    }
    return InEpMap;
}

std::map<std::string, std::string> TomlConfig::readNetTo(const cpptoml::table& netToTable) {
    std::map<std::string, std::string> OutEpMap;

    const auto agrithnmTable = netToTable.get_table("agrithnm");
    if (agrithnmTable) {
        const auto name = agrithnmTable->get_as<std::string>("name");
        const auto endPoint = agrithnmTable->get_as<std::string>("end_point");
        OutEpMap[name->c_str()] = endPoint->c_str();
        if (name && endPoint) {
            std::cout << "NetTo "<< ": " << *name << ", End Point: " << *endPoint << std::endl;
        }
    }
    const auto functionTable = netToTable.get_table("function");
    if (functionTable) {
        const auto name = functionTable->get_as<std::string>("name");
        const auto endPoint = functionTable->get_as<std::string>("end_point");
        OutEpMap[name->c_str()] = endPoint->c_str();
        if (name && endPoint) {
            std::cout << "NetTo " << ": " << *name << ", End Point: " << *endPoint << std::endl;
        }
    }
    return OutEpMap;
}

void TomlConfig::ReadConfig(const std::string& tomlpath,int ServerOrClient) {
    try {
        const auto config = cpptoml::parse_file(tomlpath);
        const auto cellTable = config->get_table("cell");
        if (cellTable) {
            const auto name = cellTable->get_as<std::string>("name");
            const auto id = cellTable->get_as<std::string>("id");

            m_cellName = name->c_str();
            m_cellId = id->c_str();

            if (name && id) {
                std::cout << "Cell: Name: " << *name << ", ID: " << *id << std::endl;
                std::shared_ptr<cpptoml::table_array> netInArray = nullptr;
                std::shared_ptr<cpptoml::table_array> netToArray = nullptr;
                switch (ServerOrClient) {
                    case 0:
                        netInArray = config->get_table_array("netin");
                        break;
                    case 1:
                        netToArray = config->get_table_array("netto");
                        break;
                    case 2:
                        netInArray = config->get_table_array("netin");
                        netToArray = config->get_table_array("netto");
                        break;
                }
                if (netInArray) {
                    std::cout << "NetIn:" << std::endl;
                    for (const auto& netInTable : *netInArray) {
                        m_InEpMap = readNetIn(*netInTable);
                    }
                }                          
                if (netToArray) {
                    std::cout << "NetTo:" << std::endl;
                    for (const auto& netToTable : *netToArray) {
                        m_outEpMap = readNetTo(*netToTable);
                    }
                }
            }
        }
        m_file = tomlpath;
    }
    catch (const cpptoml::parse_exception& e) {
        std::cerr << "Toml解析错误: " << e.what() << std::endl;
    }
}

void TomlConfig::x_ReadConfig(const std::string &tomlpath)
{
    //读取配置
    try
    {
        std::shared_ptr<cpptoml::table> gconfig = cpptoml::parse_file(tomlpath);
        std::cout << (*gconfig) << std::endl;

        //camera_svr =
        m_cellName = gconfig->get_qualified_as<std::string>("cell")->c_str();
        m_cellId = gconfig->get_qualified_as<std::string>("cell.id")->c_str();
        auto inEplist = gconfig->get_table_array_qualified("net.to")->as_table_array();
        auto toEplist = gconfig->get_table_array_qualified("net.to")->as_table_array();
        for (auto item : *inEplist)
        {
            std::string name = item->get_qualified_as<std::string>("name")->c_str();
            std::string ep = item->get_qualified_as<std::string>("end_point")->c_str();
            m_InEpMap[name] = ep;//需连接的客户端信息
        }
        for (auto item: *toEplist)
        {
            std::string name = item->get_qualified_as<std::string>("name")->c_str();
            std::string ep = item->get_qualified_as<std::string>("end_point")->c_str();
            m_outEpMap[name] = ep;//需连接的客户端信息
        }

        m_file = tomlpath;
    }
    catch (const cpptoml::parse_exception& e)
    {
        std::cerr << "Failed to parse " << tomlpath<< ": " << e.what() << std::endl;
    }
}

void TomlConfig::Reload()
{
}

