#pragma once
#include "../HttpMessage/HttpMessage.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../Route/Route.hpp"
#include <cstdio>      // for remove
#include <iostream>    // for std::cerr
#define DEFAULTERROR "www/html/errorPages/DefaultError.html"
#define DEFAULTDELETE "www/html/defaultpagedelete.html"
#define DEFAULTINDEX "www/html/indexing.html"
class HttpResponse :  public HttpMessage{
private:
    int statusCode;
    std::string reasonPhrase;
    std::string Page;
    std::set<std::string> ValidcgiExtensions;
    std::map<int, std::string> defaultErrors;
    std::map<std::string, std::string> mimeTypes;
    ssize_t totaSize;
    size_t offset;
    bool   headerSended;
    std::string query;
    

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
    HttpResponse();
    
    void ResponseGenerating(HttpRequest & request, std::map<int, std::string> &errorPages, int clientSocketId, Status& status);
    //void initResponse(const Route &route,std::map<int, std::string> &errorPage, int code,const std::string &query, const std::string UrlRequest, const std::string method);    
    std::string getMimeType(const std::string& filePath) const;
    void addHeaders();
    void UpdateStatueCode(int code);
    void handleRedirection(const Route &route);
    //void handleError(std::map<int, std::string>& errorPages);
    size_t checkIfCGI(const std::string& url);

    void HandleIndexing(std::string fullpath, std::string& uri);
    void GeneratePageIndexing(std::string& fullpath, std::string& uri, std::vector<std::string>& files);
    size_t getSendbytes();
    void addToSendbytes(size_t t);
    // void buildingHeaders();
    void buildResponseBuffer(int clientSocketId, Status& status); // this for building and set it in send syscall
};

