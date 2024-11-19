#pragma once
#include "../Route/Route.hpp"
#include "../Connection/Connection.hpp"

class Connection;
class Server {
private:
    std::string host;
    in_addr_t ip_addr;
    std::vector<int> ports;
    std::vector<int> sockets;
    std::map<int, Connection*> connections;
    std::vector<std::string> serverNames; 
    std::string serverRoot;
    std::map<std::string, Route> routes;
    std::map<int, std::string> errorPages;
    size_t clientMaxBodySize;


public:
    void serverSocketSetter(int Port, int Socket);
    std::vector<int> getSockets() const;
    //std::map<int ,int> &serverSocketGetter();
    void hostSetter(std::string host);
    std::string& hostGetter();
    void portSetter(int port);
    std::vector<int> &portGetter();
    int GetPort(int socketserver) const;
    void serverNamesSetter(std::string serverName);
    std::vector<std::string> serverNamesGetter();
    void serverRootSetter(std::string serverRoot);
    std::string serverRootGetter();
    void errorPagesSetter(int errorCode, std::string errorPage);
    std::map<int, std::string>& errorPagesGetter();
    void clientMaxBodySizeSetter(size_t clientMaxBodySize);
    size_t clientMaxBodySizeGetter();
    void addRoute(Route newRoute);
    Route& getRoute(const std::string& path);
    std::map<std::string, Route>& getRoutes();
    void addSocket(int socket);
    int SearchSockets(int id);

    void addConnection(int socket, Connection* connection);
    Connection* GetConnection(int client);
    bool removeConnection(int socket);
    
    void portEraser(int pos);
    bool hasClient(int client) const;
    void setIpaddress(std::string addr);
    in_addr_t getIpaddress();

    std::map<int, Connection*>& GetCoonections() ;
        // Server();
    // Server(const std::string& host, const std::vector<int>& ports) 
    //     : host(host), ports(ports) {}

    // bool setup(); // Sets up the server
    // bool listen(); // Starts listening for connections
    // void addNewConnection(Connection newConnection);
};