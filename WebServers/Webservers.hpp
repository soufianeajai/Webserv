#pragma once
#include "Server.hpp"

class WebServer {
private:
    std::map<int, Server> servers; // List of servers parsed from configuration int is like an ID or somthing ti identify the Server

public:
    WebServer(const std::string& configFile);
    bool initialize(); // Initializes the servers from config file
    void run(); // run the Main servers loop
    void addNewServer(Server& newServer);
    Server& getServer(int identifier);
};