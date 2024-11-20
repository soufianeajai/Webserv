#pragma once
#include "../Server/Server.hpp"

class ParsingConfig
{
    std::vector<Server> servers;
public:
    void addServer(Server& newServer);
    Server& getServer(int identifier);
    std::vector<Server>& getServers();
    ParsingConfig() {}
    bool hostCheck(std::string host);
    bool checkClientBodySize(std::string &str);
    bool containsOnlySpaces(std::string &str);
    void checkDefaultServer();
    void checkNecessary(std::ifstream& FILE);
};

ParsingConfig parsingConfig(const char *configFile);
int numberConversion(std::string &string);