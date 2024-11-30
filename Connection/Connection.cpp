#include "Connection.hpp"

time_t Connection::get_last_access_time() const
{
    return last_access_time;
}

void Connection::set_last_access_time(time_t last)
{
    last_access_time = last;
}
size_t Connection::getLimitBodySize(size_t maxSize){
    size_t temp = maxSize * MB;
    return (temp < Connection::MAX_BODY_SIZE ? temp : Connection::MAX_BODY_SIZE);
}
Connection::Connection(int fd, const sockaddr_in &acceptedAddr, size_t maxSize, struct epoll_event& epoll, int serversocket,time_t last):clientSocketId(fd), status(READING_PARSING),epollfd(epoll),last_access_time(last)
{
    limitBodySize = getLimitBodySize(maxSize);
    this->socketServer = serversocket;
    CLientAddress.sin_family = acceptedAddr.sin_family;
    CLientAddress.sin_port = acceptedAddr.sin_port;
    CLientAddress.sin_addr = acceptedAddr.sin_addr;
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int Connection::getsocketserver() const
{
    return socketServer;
}

struct epoll_event& Connection::getEpollFd()
{
    return epollfd;
}
int Connection::getClientSocketId() const{
    return clientSocketId;
}

void Connection::parseRequest(){
    uint8_t    buffer[Connection::CHUNK_SIZE];
    int     readSize = 0;
    int clientSocket = getClientSocketId();
    std::memset(buffer, 0, Connection::CHUNK_SIZE);
    readSize = recv(clientSocket, buffer, Connection::CHUNK_SIZE, MSG_DONTWAIT);
    if (readSize == 0){
        std::cerr << "Client closed the connection." << std::endl;
        close(clientSocket);
        clientSocket = -1;
        return;
    }
    else if (readSize < 0)
    {
        std::cerr << "No data to read but connection still opened " << std::endl;
        return ;
    }
    else
    {
        request.parse(buffer, readSize, limitBodySize);
        if (request.parsingCompleted())
            status = PROCESSING;
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
    if (request.getcurrentState() == PROCESS_DONE || request.errorOccured()){
        if (request.errorOccured())
        {
            std::map<State, int>::const_iterator it = request.getErrorState().find(request.getcurrentState());
            request.SetStatusCode(it->second);
        }
        status = GENARATE_RESPONSE;
    }
}


void Connection::generateResponse(std::set<std::string>& serverNamesGetter,std::map<int, std::string> &errorPages,std::string& host, uint16_t port,time_t currenttime)
{
    if (status == GENARATE_RESPONSE)
        response.ResponseGenerating(request,serverNamesGetter , errorPages, status,host,port, currenttime);
    else if (status == SENDING_RESPONSE)
        response.sendData(clientSocketId, status);
}

Status Connection::getStatus() const
{
    return status;
}

void    Connection::setStatus(Status stat)
{
    status = stat;
}

HttpRequest Connection::getRequest()
{
    return request;
}


HttpResponse Connection::getResponse()
{
    return response;
}
