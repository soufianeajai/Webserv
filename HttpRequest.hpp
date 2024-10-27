#pragma once
#include "HttpMessage.hpp"
class HttpMessage {
protected:
    std::map<std::string, std::string> headers;
    std::vector<uint8_t> body; // Raw body data
    std::string version;
    // we can just version "HTTP/1.1" , sinmon  :  400 Bad Request ...

public :
    const static unsigned int LimitRequestBody = 10485760;
    HttpMessage();
// this after validation of header , can store it in map container
    void addHeader(const std::string& key, const std::string& value);

    std::string getHeader(const std::string& key) const;

    void setBody(const std::string& bodyContent);

    std::string getBody() const;

    std::string getVersion() const;
 
    virtual std::string toString() const;
};


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

/* rules of parsing in the RFC 7230
request-line   = method SP request-target SP HTTP-version CRLF

*/