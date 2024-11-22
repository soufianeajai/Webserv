#pragma once
#include "../HttpMessage/HttpMessage.hpp"
#include "../HttpRequest/HttpRequest.hpp"
#include "../Route/Route.hpp"
#include <errno.h>
#include <sys/wait.h> // for waitpid
#include <cstdio>      // for remove
#include <iostream>    // for std::cerr
#define DEFAULTERROR "www/html/errorPages/DefaultError.html"
#define DEFAULTDELETE "www/html/defaultpagedelete.html"
#define DEFAULTINDEX "www/html/indexing.html"
extern char **environ; 
#define TIMEOUT 2
class HttpResponse :  public HttpMessage{
private:
    int statusCode;
    std::string reasonPhrase;
    std::string Page;
    std::map<int, std::string> defaultErrors;
    std::map<std::string, std::string> mimeTypes;
    ssize_t totaSize;
    size_t offset;
    bool   headerSended;
    bool cgi;
    std::vector<char*> envVars;
    std::string cgiOutput; // delete
    std::string PathCmd;
    std::string PATH_INFO;
    std::string PWD;
    int pipefd[2];
    pid_t pid;
    time_t currenttime; 

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
    //bool getCgi() const;
    //pid_t getPid() const;
    //int getpipe() const;
    //size_t getOffset();
    void ResponseGenerating(HttpRequest & request, std::map<int, std::string> &errorPages, 
                 Status& status,std::string& host, uint16_t port, time_t currenttime);
    void handleRequest(std::string& host, uint16_t port,HttpRequest & request);
    //void HttpResponse::resolveRequestPath(HttpRequest& request, Route& route, std::string& uri, std::string& host, uint16_t port)
    std::string getMimeType(const std::string& filePath) const;
    void addHeaders(std::string size, std::string mime);
    void UpdateStatueCode(int code);
    void handleRedirection(const Route &route);
    //void handleError(std::map<int, std::string>& errorPages);
    void checkIfCGI(HttpRequest& request, std::string& path, std::map<std::string, std::string> ExtensionsConfig, std::string& uri,const std::string& host,const std::string& port);
    void HandleIndexing(std::string fullpath, std::string& uri);
    void GeneratePageIndexing(std::string& fullpath, std::string& uri, std::vector<std::string>& files);
    //size_t getSendbytes();
    int executeCGI();
    int parentProcess();
    void sendCgi(int clientSocketId, Status& status);
    void createEnvChar(HttpRequest& request, std::string& uri,const std::string& host,const std::string& port);
    void CheckExistingInServer();
    void GetFullPathCmd(const std::string& ext);
    void sendData(int clientSocketId, Status& status); // this for building and set it in send syscall
};

