#pragma once
#include "HttpMessage.hpp"

class HttpResponse :  public HttpMessage{
private:
    int statusCode;
    std::string reasonPhrase;
    std::map<int, std::string> Pages;
    
public:
    HttpResponse();
    void GenerateResponse(const Request& request);
    void setReasonPhrase(int statusCode);
    void LoadErrorPage();
};