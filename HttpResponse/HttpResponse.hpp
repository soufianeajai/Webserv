#pragma once
#include "HttpMessage.hpp"

class HttpResponse :  public HttpMessage{
private:
    int statusCode;
    std::string reasonPhrase;
    std::map<int, std::string> Pages;
    
public:
    HttpResponse(int code, std::string url);
    void LoadPage();

    std::vector<uint8_t> buildResponseBuffer(); // this for building and set it in send syscall
};

// send data !!
bool sendData(int fd, const std::vector<uint8_t>& data)
{
    size_t totalBytesSent = 0;
    size_t dataSize = data.size();

    while (totalBytesSent < dataSize)
    {
        ssize_t bytesSent = send(fd, &data[totalBytesSent], dataSize - totalBytesSent, 0);
        if (bytesSent <= 0) {
            return false; // Error occurred, stop sending and return false
        }
        totalBytesSent += bytesSent;
    }
    return true;
}