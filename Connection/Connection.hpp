#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse/HttpResponse.hpp"
#include "../Server/Server.hpp"


enum Status {INITIAL, READING_PARSING, PROCESSING, GENARATE_RESPONSE, SENDING_RESPONSE, DONE, ERROR};
class Connection {
private:
    sockaddr_in CLientAddress;
    int clientSocketId;
    HttpRequest request;
    HttpResponse response;
    size_t bodySize;
    Status status;
public:
    static const size_t MAX_BODY_SIZE = 10 * 1024 * 1024;
    static const size_t CHUNK_SIZE = 8 * 1024;
    Connection();
    Connection(int fd, const sockaddr_in &acceptedAddr, size_t maxSize);
    void readIncomingData(std::map<std::string, Route>& routes); // read data from socket clientSocketId in the buffer requestBuffer until the reaching the limit size of the buffer or the reading is ended.
    void parseRequest();
    // void writedata();
    void closeConnection();
    int getClientSocketId() const;
    void generateResponse(std::map<int, std::string> &errorPages, std::map<std::string, Route>& routes);
    HttpRequest getRequest() const;
    Status getStatus() const;
    void    setStatus(Status stat);
};