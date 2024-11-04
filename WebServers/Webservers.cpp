#include "Webservers.hpp"
const  std::string globalRoot="/home/$(USER)/MAIN_webserv/docs/";
void WebServer::addServer(Server& newServer, int identifier) {
    this->servers[identifier] = newServer;
}
Server& WebServer::getServer(int identifier) {
    return this->servers[identifier];
}
std::map<int, Server>& WebServer::getServers() {
    return this->servers;
}