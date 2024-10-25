#pragma once
#include "HttpMessage.hpp"

enum HttpMethod {
    GET,
    POST,
    DELETE,
    UNKNOWN
};

class HttpRequest :  public HttpMessage{
private:
    HttpMethod method;
    std::string url;
public:
    HttpRequest();

    void setMethod(const std::string& methodStr) ;

    HttpMethod getMethod() const ;

    void setUrl(const std::string& url) ;

    std::string getUrl() const ;
    //assembling the pieces (method version header and body)
    // override toString to include request line
    std::string toString() const override ;


    //TODO: need parsing request from fd  ,  POSSIBLE CREATE ANOTHER CLASS HttpParser just for parsing 

};