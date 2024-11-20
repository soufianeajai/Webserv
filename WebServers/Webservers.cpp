#include "Webservers.hpp"

void WebServer::addServer(Server& newServer) {
    this->servers.push_back(newServer);
}
Server& WebServer::getServer(int identifier) {
    return this->servers[identifier];
}
std::vector<Server>& WebServer::getServers() {
    return this->servers;
}
