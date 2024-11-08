#pragma once
#include "../Server/Server.hpp"


class WebServer {
private:
    std::vector<Server> servers; // List of servers parsed from configuration int is like an ID or somthing ti identify the Server

public:
    void addServer(Server& newServer);
    Server& getServer(int identifier);
    std::vector<Server>& getServers();
    // WebServer(const std::string& configFile);
    // bool initialize(); // Initializes the servers from config file
    // void run(); // run the Main servers loop
    // void addNewServer(Server& newServer);
    // Server& getServer(int identifier);
};