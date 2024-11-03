#pragma once
#include "../WebServers/Webservers.hpp"  

class ParsingConfig
{
public:
    WebServer webServer;
    ParsingConfig() {}
};



ParsingConfig parsingConfig(char *configFile);
