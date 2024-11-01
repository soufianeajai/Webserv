#pragma once
#include "../WebServers/Webservers.hpp"  

class ParsingConfig
{
public:
    std::string configFile;
    WebServer webServer;
    ParsingConfig() {}
};



ParsingConfig parsingConfig(const std::string& configFile);
