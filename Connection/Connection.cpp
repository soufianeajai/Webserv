#include "Connection.hpp"
void Connection::setClientAddr(const sockaddr_in &acceptedAddr)
{
    CLientAddress.sin_family = acceptedAddr.sin_family;
    CLientAddress.sin_port = acceptedAddr.sin_port;
    CLientAddress.sin_addr = acceptedAddr.sin_addr;
}

Connection::Connection(int fd):clientSocketId(fd), connectionStatus(INITIAL), bodySize(0)
 {
    fcntl(fd, F_SETFL, O_NONBLOCK);
 }


void    Connection::readIncomingData()
{
    uint8_t    buffer[Connection::CHUNK_SIZE];
    int     readSize = 0;
    int clientSocket = this->getClientSocketId();

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
    else
        this->request.parse(buffer, readSize);

    if (this->request.parsingCompleted() || this->request.errorOccured()) 
    {
        // parsing completed and we can process the request and work on the response.
    }
}

void Connection::generateResponse()
{
    
}