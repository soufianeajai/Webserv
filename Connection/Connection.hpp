#pragma once
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <arpa/inet.h>
#include <fcntl.h>

class Connection {
private:
    sockaddr_in CLientAddress;
    int clientSocketId;
    HttpRequest request;
    HttpResponse response;
    size_t bodySize;
    enum Status {INITIAL, READING, PARSING, PROCESSING, GENARATE_RESPONSE, WRITING, DONE} connectionStatus;

public:
    static const size_t MAX_BODY_SIZE = 10 * 1024 * 1024;
    static const size_t CHUNK_SIZE = 8 * 1024;
    Connection(int fd);
    void readIncomingData(); // read data from socket clientSocketId in the buffer requestBuffer until the reaching the limit size of the buffer or the reading is ended.
    void parseRequest();
    void generateResponse();
    void writedata();
    void closeConnection();
    int getClientSocketId() const;
    void setclientSocketId(int id);
    void setClientAddr(const sockaddr_in &acceptedAddr);
};