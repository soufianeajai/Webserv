#pragma once
#include "../HttpRequest/HttpRequest.hpp"
#include "../Route/Route.hpp"
#define DEFAULTERROR "www/html/errorPages/DefaultError.html"
#define DEFAULTINDEX "www/html/indexing.html"
#define DEFAULTREDIRECT "www/html/redirection/newindex.html"
#define SESSION "/session"
extern char **environ;

#define TIMEOUT 3
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
    std::vector<uint8_t> cgiOutput; // delete
    std::string PathCmd;
    std::string PATH_INFO;
    std::string PWD;
    int pipefd[2];
    pid_t pid;
    time_t currenttime;
    bool ChildFInish;
    std::string Cookies;
public:
    HttpResponse();

    bool getCgi() const;
    pid_t getPid() const;
    //int getpipe() const;
    //size_t getOffset();
    void ResponseGenerating(HttpRequest & request,std::set<std::string>& serverNamesGetter, std::map<int, std::string> &errorPages, 
                 Status& status,std::string& host, uint16_t port, time_t currenttime);
    void handleRequest(std::string& host, uint16_t port,HttpRequest & request);
    //void HttpResponse::resolveRequestPath(HttpRequest& request, Route& route, std::string& uri, std::string& host, uint16_t port)
    std::string getMimeType(const std::string& filePath) const;
    void UpdateStatueCode(int code);
    void handleRedirection(const Route &route);
    //void handleError(std::map<int, std::string>& errorPages);
    void checkIfCGI(std::string& path, std::map<std::string, std::string> ExtensionsConfig );
    void HandleIndexing(std::string fullpath, std::string& uri);
    void GeneratePageIndexing(std::string& fullpath, std::string& uri, std::vector<std::string>& files);
    //size_t getSendbytes();
    int executeCGI();
    int parentProcess();
    void sendCgi(int clientSocketId, Status& status);
    void createEnvChar(HttpRequest& request, std::string& uri,const std::string& host,const std::string& port);
    void CheckExistingInServer();
    void sendData(int clientSocketId, Status& status); // this for building and set it in send syscall
    void SendHeaders(int clientSocketId, Status& status,std::vector<uint8_t>& heads);
    void ExtractHeaders();
    void handleCookie(HttpRequest & request);
    void handleServerName(std::set<std::string>& serverNamesGetter, std::string hostrequest,std::string host);
};
std::string intToString(size_t number);
