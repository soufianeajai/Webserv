#include "Server.hpp"


void Server::hostSetter(std::string _host) {
    this->host = _host;
}
std::string Server::hostGetter() {
    return this->host;
}
void Server::portSetter(int _port) {
    this->ports.push_back(_port);
}
std::vector<int> &Server::portGetter() {
    return this->ports;
}
void Server::serverNamesSetter(std::string _serverName) {
    this->serverNames.push_back(_serverName);
}
std::vector<std::string> Server::serverNamesGetter() {
    return this->serverNames;
}
void Server::serverRootSetter(std::string _serverRoot) {
    this->serverRoot = _serverRoot;
}
std::string Server::serverRootGetter() {
    return this->serverRoot;
}
void Server::errorPagesSetter(int _errorCode, std::string _errorPage) {
    this->errorPages[_errorCode] = _errorPage;
}
std::map<int, std::string> Server::errorPagesGetter() {
    return this->errorPages;
}
void Server::clientMaxBodySizeSetter(size_t _clientMaxBodySize) {
    this->clientMaxBodySize = _clientMaxBodySize;
}
size_t Server::clientMaxBodySizeGetter() {
    return this->clientMaxBodySize;
}
void Server::addRoute(Route newRoute) {
    this->routes[newRoute.getPath()] = newRoute;
}
Route& Server::getRoute(const std::string& path) {
    return this->routes[path];
}
std::map<std::string, Route>& Server::getRoutes() {
    return this->routes;
}
