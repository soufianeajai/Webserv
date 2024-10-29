#pragma once
#include "Route.hpp"
#include "Connection.hpp"

class Server {
private:
    std::string host;
    std::vector<int> ports;
    std::vector<std::string> serverNames; 
    std::string serverRoot;
    std::map<int, Connection*> connections;
    std::map<std::string, Route> routes;
    std::map<int, std::string> errorPages;
    size_t clientMaxBodySize;
    bool isDefault;

public:
    Server(const std::string& host, const std::vector<int>& ports) 
        : host(host), ports(ports) {}

    bool setup(); // Sets up the server
    bool listen(); // Starts listening for connections
    void addNewConnection(Connection newConnection);
};