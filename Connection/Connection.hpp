#pragma once
#define REQTIMEOUT "HTTP/1.1 408 Request Timeout\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"
#include "../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse/HttpResponse.hpp"
#include "../Server/Server.hpp"
#include <sys/epoll.h>
#include <ctime>
#define MB 1024 * 1024
#define KB 1024
class Connection {
private:
    
    sockaddr_in CLientAddress;
    int clientSocketId;
    HttpRequest  request;
    HttpResponse response;
    size_t limitBodySize;
    Status status;
    struct epoll_event epollfd;
    int socketServer;
    time_t last_access_time;

public:
    static const ssize_t MAX_BODY_SIZE = 100 * MB;
    static const ssize_t CHUNK_SIZE = 8 * KB;
    Connection();
    Connection(int fd, const sockaddr_in &acceptedAddr, size_t maxSize,struct epoll_event& epollfd, int socketserver, time_t last_access_time);
    struct epoll_event& getEpollFd();
    void readIncomingData(std::map<std::string, Route>& routes); // read data from socket clientSocketId in the buffer requestBuffer until the reaching the limit size of the buffer or the reading is ended.
    void parseRequest();
    void SendData(const std::vector<uint8_t>& buffer);
    // void writedata();
    int getClientSocketId() const;
    void generateResponse(std::set<std::string>& serverNamesGetter,std::map<int, std::string> &errorPages, std::string& host, uint16_t port,time_t currenttime);
    HttpRequest getRequest();
    HttpResponse getResponse();
    Status getStatus() const;
    void    setStatus(Status stat);
    int getsocketserver() const;
    time_t get_last_access_time() const ;
    void set_last_access_time(time_t last);
    size_t getLimitBodySize(size_t maxSize);
};