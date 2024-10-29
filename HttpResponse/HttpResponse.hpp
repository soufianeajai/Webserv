#pragma once
#include "HttpMessage.hpp"

class HttpResponse :  public HttpMessage{
private:
    int statusCode;
    std::string reasonPhrase;
    std::map<int, std::string> errorPages;
public:
    HttpResponse();
    
    void setStatusCode(int code);

    std::string getReasonPhrase() const;

    std::string toString() const override;

    void LoadErrorPage();
};