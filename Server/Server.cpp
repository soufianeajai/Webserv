#include "Server.hpp"

int Server::getSocketsrv() const
{
    return Socketsrv;
}
void Server::setSocketsrv(int socket)
{
    Socketsrv = socket;
}
void Server::hostSetter(std::string _host) {
    this->host = _host;
}
std::string Server::hostGetter() const{
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
std::vector<std::string> Server::serverNamesGetter() const{
    return this->serverNames;
}
void Server::serverRootSetter(std::string _serverRoot) {
    this->serverRoot = _serverRoot;
}
std::string Server::serverRootGetter() const{
    return this->serverRoot;
}
void Server::errorPagesSetter(int _errorCode, std::string _errorPage) {
    this->errorPages[_errorCode] = _errorPage;
}
std::map<int, std::string> Server::errorPagesGetter() const{
    return this->errorPages;
}
void Server::clientMaxBodySizeSetter(size_t _clientMaxBodySize) {
    this->clientMaxBodySize = _clientMaxBodySize;
}
size_t Server::clientMaxBodySizeGetter() const{
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
