#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../HttpResponse/HttpResponse.hpp"


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
    Connection();
    Connection(int fd, const sockaddr_in &acceptedAddr, size_t maxSize);
    void readIncomingData(); // read data from socket clientSocketId in the buffer requestBuffer until the reaching the limit size of the buffer or the reading is ended.
    // void parseRequest();
    // void writedata();
    void closeConnection();
    int getClientSocketId() const;
    void generateResponse(std::map<int, std::string> &errorPages, std::map<std::string, Route>& routes) const;
};