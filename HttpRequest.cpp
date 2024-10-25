#include "HttpRequest.hpp"

HttpRequest::HttpRequest():method(UNKNOWN),url("/") {}

    void HttpRequest::setMethod(const std::string& methodStr) {
        if (methodStr == "GET") method = GET;
        else if (methodStr == "POST") method = POST;
        else if (methodStr == "DELETE") method = DELETE;
        else method = UNKNOWN;
    }

    HttpMethod HttpRequest::getMethod() const { return method; }

    void HttpRequest::setUrl(const std::string& url) {
        this->url = url;
    }

    std::string HttpRequest::getUrl() const {
        return url;
    }

    //assembling the pieces (method version header and body)
    // override toString to include request line
    std::string HttpRequest::toString() const  {
        std::stringstream ss;
        ss << (method == GET ? "GET" : method == POST ? "POST" : "DELETE") 
           << " " << url << " " << version << "\r\n";
        ss << HttpMessage::toString();  // add also headers and body from base
        return ss.str();
    }
