#include "Connection.hpp"

Connection::Connection(){}

Connection::Connection(int fd, const sockaddr_in &acceptedAddr, size_t maxSize):clientSocketId(fd), bodySize(maxSize), status(READING_PARSING)
 {
    CLientAddress.sin_family = acceptedAddr.sin_family;
    CLientAddress.sin_port = acceptedAddr.sin_port;
    CLientAddress.sin_addr = acceptedAddr.sin_addr;
    fcntl(fd, F_SETFL, O_NONBLOCK);

    // request = new HttpRequest();
    // response = new HttpResponse();
    (void)bodySize;
 }

int Connection::getClientSocketId() const{
    return clientSocketId;
}
void Connection::closeConnection(){

}
void Connection::parseRequest(){
    uint8_t    buffer[Connection::CHUNK_SIZE];
    int     readSize = 0;
    int clientSocket = getClientSocketId();
    memset(buffer, 0, Connection::CHUNK_SIZE);
    readSize = recv(clientSocket, buffer, Connection::CHUNK_SIZE, MSG_DONTWAIT);
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
        request.parse(buffer, readSize);
        if (request.parsingCompleted())
            status = PROCESSING;
        if (request.errorOccured())
            status = ERROR;
    }
}

void    Connection::readIncomingData(std::map<std::string, Route>& routes)
{
    std::map<std::string, std::string> formFields;
    if (status == READING_PARSING)
        parseRequest();
    if (status == PROCESSING){
        request.process(routes);
    }
    if (request.getcurrentState() == PROCESS_DONE)
    {
        status = GENARATE_RESPONSE;
    }
}

void Connection::generateResponse(std::map<int, std::string> &errorPages)
{
    std::cout << "generate response : "<< request.getcurrentState() <<"\nurl route : -"<<request.getUri()<<"__"<<request.getQuery()<<"__"<<request.getCurrentRoute().getPath()<<"\nmethod request: "<<  request.getMethod()<<std::endl; 
    response.ResponseGenerating(request.getCurrentRoute(), errorPages, request.GetStatusCode(), request.getQuery(), request.getUri(), request.getMethod());
}

Status Connection::getStatus() const{
    return status;
}

void    Connection::setStatus(Status stat){
    status = stat;
}

HttpRequest Connection::getRequest()
{
    return request;
}