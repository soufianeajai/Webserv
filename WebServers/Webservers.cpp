#include "Webservers.hpp"

void WebServer::addServer(Server& newServer) {
    servers.push_back(newServer);
}

std::vector<Server>& WebServer::getServers() {
    return this->servers;
}