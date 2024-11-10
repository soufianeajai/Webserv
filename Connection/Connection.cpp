#include "Connection.hpp"
#define DEFAULTERROR "<html><body><h1>Default Error Page</h1></body></html>"
Connection::Connection(){}

Connection::Connection(int fd, const sockaddr_in &acceptedAddr, size_t maxSize):clientSocketId(fd), bodySize(maxSize), status(INITIAL)
 {
    CLientAddress.sin_family = acceptedAddr.sin_family;
    CLientAddress.sin_port = acceptedAddr.sin_port;
    CLientAddress.sin_addr = acceptedAddr.sin_addr;
    fcntl(fd, F_SETFL, O_NONBLOCK);
 }

int Connection::getClientSocketId() const{
    return clientSocketId;
}
void Connection::closeConnection(){

}
void Connection::parseRequest(){
    uint8_t    buffer[Connection::CHUNK_SIZE];
    int     readSize = 0;
    int clientSocket = this->getClientSocketId();
    (void)bodySize;
    memset(buffer, 0, Connection::CHUNK_SIZE);
    readSize = recv(clientSocket, buffer, Connection::CHUNK_SIZE, MSG_DONTWAIT);
    if (readSize == 0)
    {
        std::cout << "no data to read from socket or connection is closed " << clientSocket << std::endl;
        this->closeConnection();
        return ;
    }
    else if (readSize < 0)
    {
        std::cerr << "Error receiving data: " << clientSocket << " " << strerror(errno) << std::endl;
        this->closeConnection();
        return ;
    }
    else{
        this->request.parse(buffer, readSize);
        if (this->request.parsingCompleted())
            status = PROCESSING;
    }
}
void    Connection::readIncomingData(std::map<std::string, Route>& routes)
{
    std::map<std::string, std::string> formFields;
    while (status != GENARATE_RESPONSE && status != ERROR) {
        switch (status)
        {
            case READING_PARSING: parseRequest(); break;
            case PROCESSING: 
                formFields = this->request.process(routes); 
                status = GENARATE_RESPONSE;
                break;
            case ERROR: 
            default:
                break;
        }
    }
}

void Connection::generateResponse(std::map<int, std::string> &errorPages, std::map<std::string, Route>& routes)
{
    Route route;
    std::string errorpage;
    int code =  request.GetStatusCode();
    if (code > 199 &&  code < 400)
    {
        std::map<std::string, Route>::iterator routeIt = routes.find(request.getUri()); // detect url which route is ...
        if (routeIt != routes.end())
            route = routeIt->second;
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
    response.initResponse(route, errorpage, code, request.getQuery(), request.getUri(), request.getMethod());
}
