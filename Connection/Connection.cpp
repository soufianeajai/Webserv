#include "Connection.hpp"

Connection::Connection():bodySize(0){}

Connection::Connection(int fd, const sockaddr_in &acceptedAddr, size_t maxSize, struct epoll_event& epoll, int serversocket):clientSocketId(fd), bodySize(maxSize), status(READING_PARSING),epollfd(epoll)
{
    std::cout << "conneciton: "<<serversocket<<"\n";
    this->socketServer = serversocket;
    CLientAddress.sin_family = acceptedAddr.sin_family;
    CLientAddress.sin_port = acceptedAddr.sin_port;
    CLientAddress.sin_addr = acceptedAddr.sin_addr;
    fcntl(fd, F_SETFL, O_NONBLOCK);

    // request = new HttpRequest();
    // response = new HttpResponse();
    (void)bodySize;
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
    }
}

void    Connection::readIncomingData(std::map<std::string, Route>& routes)
{
//    std::cout << "state in readIncomingData " << status << std::endl;
    
    std::map<std::string, std::string> formFields;
    if (status == READING_PARSING)
        parseRequest();
    if (status == PROCESSING){
        request.process(routes);
    }
    if (request.getcurrentState() == PROCESS_DONE || request.errorOccured())
        status = GENARATE_RESPONSE;
    if (status == GENARATE_RESPONSE)
    {
        std::cout << "----> " << request.GetStatusCode() << std::endl;
        // if (!buffer.empty())
        //     status = SENDING_RESPONSE;
        // std::cout <<"\n\nResponseGenerating : ";
        // for(size_t i = 0; i <  buffer.size();i++)
        //     std::cout <<buffer[i];
        // else
        //     std::cout << "\n2222waaaaaaaaaaaaaaaaa\n";
    }
}


// void Connection::SendData(const std::vector<uint8_t>& buffer)
// {
//     ssize_t SentedBytes = 0; // we have  it is :  response.getSendbytes()
//     size_t n = Connection::CHUNK_SIZE;
//     // max sending : Connection::CHUNK_SIZE
//     if (status == SENDING_RESPONSE)
//     {
//         if (buffer.size() < Connection::CHUNK_SIZE)
//             n =  buffer.size();
//         //std::cout << "buffer length : "<<n <<" "<<response.getSendbytes()<< " "<<buffer[0]<<std::endl;
//         SentedBytes = send(clientSocketId, &buffer[response.getSendbytes()], n, MSG_NOSIGNAL);
//         if (SentedBytes < 0)
//         { 
//             std::cerr << "Send error: " << std::endl;
//             status = DONE;  // handle error as needed
//         }
//         if (SentedBytes > 0)
//         {
//             response.addToSendbytes(SentedBytes);
//             //std::cout << "\n\nstatus2 : "<<SentedBytes<<"   "<<buffer.size()<<"\n\n";
//         }
//         //std::cout << "::::::::"<<response.getSendbytes()<<std::endl;
//         if (response.getSendbytes() == buffer.size()) 
//         {
//             status = DONE;

//              std::cout << "\n\nstatus3\n\n";
//         }
//         // if else update state to GENARATE_RESPONSE 
//     }

// }
void Connection::generateResponse(std::map<int, std::string> &errorPages,std::string& host, uint16_t port)
{
    response.ResponseGenerating(request, errorPages, clientSocketId, status,host,port);
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


HttpResponse Connection::getResponse()
{
    return response;
}
