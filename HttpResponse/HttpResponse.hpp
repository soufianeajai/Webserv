#pragma once
#include "HttpMessage.hpp"

class HttpResponse :  public HttpMessage{
private:
    int statusCode;
    std::string reasonPhrase;
    std::map<int, std::string> Pages;
    //std::map<std::string, std::map<std::string, std::string>> sessions;
    std::map<std::string, std::string> mimeTypes;
    std::set<std::string> cgiExtensions = { ".php", ".py"};


    
public:
   HttpResponse::HttpResponse(const HttpRequest &request, const Server &server);
    void LoadPage();

    void buildingHeaders();
    std::vector<uint8_t> buildResponseBuffer(); // this for building and set it in send syscall
};

