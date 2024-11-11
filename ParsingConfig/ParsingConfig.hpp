#pragma once
#include "../WebServers/Webservers.hpp"  

class ParsingConfig
{
public:
    WebServer webServer;
    ParsingConfig() {}
    bool hostCheck(std::string host);
    bool checkClientBodySize(std::string &str);
    bool containsOnlySpaces(std::string &str);
};

ParsingConfig parsingConfig(const char *configFile);
int numberConversion(std::string &string);