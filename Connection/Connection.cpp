#include "Connection.hpp"
#define DEFAULTERROR "<html><body><h1>Default Error Page</h1></body></html>"
Connection::Connection(){}

Connection::Connection(int fd, const sockaddr_in &acceptedAddr, size_t maxSize):clientSocketId(fd), bodySize(maxSize), status(READING_PARSING)
 {
    CLientAddress.sin_family = acceptedAddr.sin_family;
    CLientAddress.sin_port = acceptedAddr.sin_port;
    CLientAddress.sin_addr = acceptedAddr.sin_addr;
    fcntl(fd, F_SETFL, O_NONBLOCK);

    request = new HttpRequest();
    response = new HttpResponse();

 }

int Connection::getClientSocketId() const{
    return clientSocketId;
}
void Connection::closeConnection(){

}
void Connection::parseRequest(){
        std::cout << "----- " << request->getcurrentState() << std::endl;

    uint8_t    buffer[Connection::CHUNK_SIZE];
//    uint8_t    globalBuffer[Connection::MAX_BODY_SIZE];
    int     readSize = 0;
    int clientSocket = getClientSocketId();
    (void)bodySize;
    memset(buffer, 0, Connection::CHUNK_SIZE);
    readSize = recv(clientSocket, buffer, Connection::CHUNK_SIZE, MSG_DONTWAIT);
     std::cout << "readSize " << readSize << std::endl;
    if (readSize == 0)
    {
    //    std::cout << " Client closed the connection" << std::endl;
        closeConnection();
        return;
    }
    else if (readSize < 0)
    {
        std::cerr << " no data to read but connection still opened " << std::endl;
        return ;
    }
    else
    {
        request->parse(buffer, readSize);
        if (request->parsingCompleted())
            status = PROCESSING;
        if (request->errorOccured())
            status = ERROR;
    }
}

void    Connection::readIncomingData(std::map<std::string, Route>& routes)
{
    std::map<std::string, std::string> formFields;
    if (status == READING_PARSING)
        parseRequest();
    if (status == PROCESSING)
        request->process(routes);
    if (request->getcurrentState() == PROCESS_DONE)
    {
        status = GENARATE_RESPONSE;
    }
}

void Connection::generateResponse(std::map<int, std::string> &errorPages, std::map<std::string, Route>& routes)
{
    Route route;
    std::string errorpage;
    int code =  request->GetStatusCode();
    if (code > 199 &&  code < 400)
    {
        
         size_t lastSlashPos = request->getUri().find("/");
         std::cout << "lastslash[pos] : "<<lastSlashPos << ", url: "<< request->getUri();
        std::map<std::string, Route>::iterator it = routes.begin();
        while (it != routes.end()) {
            std::cout << "\nKey: " << it->first << ", Route destination: " << it->second.getPath() << std::endl;

            ++it;
        }
        // std::cout <<"hhhhhhhhhhhhhhhhhh->  request->getUri() : "<<request->getUri()<<"\n\n";
        // std::map<std::string, Route>::iterator routeIt = routes.find(request->getUri()); // detect url which route is ...
        // if (routeIt != routes.end())
        // {
        //     route = routeIt->second;
        //     route.setPath(routeIt->first);
        //     std::cout << "hhhhhhhhhhhhhhhhhhhhhhhhhh";
        // }
        // if url is file from a path how can i know that -> we need to cut url
    }
    else
    {
        std::map<int, std::string>::iterator it = errorPages.find(code);// trust path from configfile
        if (it != errorPages.end())
            errorpage = it->second;
        else
            errorpage = DEFAULTERROR;
    }
    //std::cout << "generate response : "<< status<<"\nurl route : -"<<request->getUri()<<":"<<route.getPath()<<"\nmethod request: "<<  request->getMethod()<<std::endl; 
    //response->initResponse(route, errorpage, code, request->getQuery(), request->getUri(), request->getMethod());
}

Status Connection::getStatus() const{
    return status;
}

void    Connection::setStatus(Status stat){
    status = stat;
}


HttpRequest* Connection::getRequest()
{
    return request;
}