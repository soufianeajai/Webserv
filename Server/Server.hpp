#pragma once
#include "../Route/Route.hpp"
// #include "Connection.hpp"

class Server {
private:
    std::string host;
    std::vector<int> ports;
    std::vector<std::string> serverNames; 
    std::string serverRoot;
    // std::map<int, Connection*> connections;
    std::map<std::string, Route> routes;
    std::map<int, std::string> errorPages;
    size_t clientMaxBodySize;


public:
    void hostSetter(std::string host);
    std::string hostGetter();
    void portSetter(int port);
    std::vector<int> &portGetter();
    void serverNamesSetter(std::string serverName);
    std::vector<std::string> serverNamesGetter();
    void serverRootSetter(std::string serverRoot);
    std::string serverRootGetter();
    void errorPagesSetter(int errorCode, std::string errorPage);
    std::map<int, std::string> errorPagesGetter();
    void clientMaxBodySizeSetter(size_t clientMaxBodySize);
    size_t clientMaxBodySizeGetter();
    void addRoute(Route newRoute);
    Route& getRoute(const std::string& path);

    std::map<std::string, Route>& getRoutes();
    // Server();
    // Server(const std::string& host, const std::vector<int>& ports) 
    //     : host(host), ports(ports) {}

    // bool setup(); // Sets up the server
    // bool listen(); // Starts listening for connections
    // void addNewConnection(Connection newConnection);
};