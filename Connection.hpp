#pragma once
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <cstring>
#include "Buffer.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"


class Connection {
private:
    int clientSocketId;
    Buffer requestBuffer; 
    Buffer responseBuffer; 
    HttpRequest request;
    HttpResponse response;
    enum Status {INITIAL, READING, PARSING, PROCESSING, WRITING, DONE} connectionStatus;

public:
    Connection(int fd): clientSocketId(fd), connectionStatus(INITIAL){}
    void readIncomingData(); // read data from socket clientSocketId in the buffer requestBuffer until the reaching the limit size of the buffer or the reading is ended.
    void parseRequest();
    void generateResponse();
    void writedata();
    void clearConnection();
};