#include "Server.hpp"
#include "../ParsingConfig/ParsingConfig.hpp"

int Server::SearchSockets(int id)
{
    for (size_t i = 0; i < sockets.size(); ++i)
    {
        if (sockets[i] == id) 
            return i;
    }
    return -1;
}
void Server::serverSocketSetter(int Port, int Socket){
    ports.push_back(Port);
    sockets.push_back(Socket);

    std::cout <<"size ::"<<sockets.size()<<"\n";
}

bool Server::hasClient(int client) const
{
    return connections.find(client) != connections.end();
}

Connection* Server::GetConnection(int socket)
{
    if (connections.count(socket) > 0)
        return connections[socket];
    else
        return NULL;
}

bool Server::removeConnection(int socket)
{
    if (connections.count(socket) > 0) 
    {
        delete connections[socket];  
        connections.erase(socket);   
        return true;   
    }else 
        return false; 
}

void Server::addConnection(int socket, Connection* connection)
{
    connections[socket] = connection;
}


int Server::GetPort(int socketserver) const
{

    int port = ports[0];
    for(size_t i = 0;i < sockets.size();i++)
    {
        std::cout << "socket: "<<sockets.size()<<" "<<sockets[i]<<" port: "<<ports[i]<<" "<<socketserver<<"\n";
        if (sockets[i] == socketserver)
            port = ports[i];

    }
    return (port);
}

std::vector<int> Server::getSockets() const
{
    return sockets;
}

void Server::hostSetter(std::string _host) {
    this->host = _host;
}
std::string& Server::hostGetter() {
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
std::map<int, std::string>& Server::errorPagesGetter() {
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

void Server::addSocket(int socket)
{
    sockets.push_back(socket);
}
// void Server::serverSocketSetter(int Port, int Socket) {
//     this->ServersSocket[Port] = Socket;
// }
// std::map<int ,int> &Server::serverSocketGetter() {
//     return this->ServersSocket;
// }
void Server::portEraser(int pos) {
    this->ports.erase(this->ports.begin() + pos);
}
void Server::setIpaddress(std::string host) {
    std::stringstream ss(host);
    std::string to;
    std::vector<std::string> arr;
    while (getline(ss, to, '.'))
    {
        if (!to.empty())
            arr.push_back(to);
    }
    this->ip_addr = ((numberConversion(arr[0]) << 24) | (numberConversion(arr[1]) << 16)
                    | (numberConversion(arr[2]) << 8) | (numberConversion(arr[3])));
    this->ip_addr = htonl(ip_addr);
}
in_addr_t Server::getIpaddress() {
    return this->ip_addr;
}