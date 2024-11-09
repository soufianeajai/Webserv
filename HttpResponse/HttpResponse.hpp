#pragma once
#include "../HttpMessage/HttpMessage.hpp"

class HttpResponse :  public HttpMessage{
private:
    int statusCode;
    std::string reasonPhrase;
    std::map<int, std::string> Pages;
    //std::map<std::string, std::map<std::string, std::string>> sessions;
    std::map<std::string, std::string> mimeTypes;
    
    

    // cgi
    /* example full url possible
     /cgi/script.php/test/more/path?param1=value1&param2=value2
    scriptName = "/cgi/script.php";
    pathInfo = "/test/more/path";
    queryString = "param1=value1&param2=value2";
    REQUEST_METHOD from request status line
    CONTENT_TYPE and CONTENT_LENGTH: If it’s a POST request
    SERVER_PROTOCOL: Set to version from httpmessage :HTTP/1.1
    GATEWAY_INTERFACE: CGI version, typically set to CGI/1.1.
    REMOTE_ADDR and REMOTE_PORT: Set to the client’s IP address and port if available.
    SERVER_NAME and SERVER_PORT: Server’s hostname and port.
    
    */
    
public:
//    HttpResponse::HttpResponse(const HttpRequest &request, const Server &server);
    void LoadPage();

    // void buildingHeaders();
    std::vector<uint8_t> buildResponseBuffer(); // this for building and set it in send syscall
};

