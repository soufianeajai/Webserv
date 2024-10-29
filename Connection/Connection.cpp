#include "Connection.hpp"
#include <sys/socket.h>

void    Connection::readIncomingData(Connection& c)
{
    uint8_t    buffer[Connection::CHUNK_SIZE];
    int     readSize = 0;
    int clientSocket = c.getClientSocketId();

    memset(buffer, 0, Connection::CHUNK_SIZE);
    readSize = recv(clientSocket, buffer, Connection::CHUNK_SIZE, MSG_DONTWAIT);
    if (readSize == 0)
    {
        std::cout << "no data to read from socket or connection is closed " << clientSocket << std::endl;
        c.closeConnection();
        return ;
    }
    else if (readSize < 0)
    {
        std::cerr << "Error receiving data: " << clientSocket << " " << strerror(errno) << std::endl;
        c.closeConnection();
        return ;
    }
    else
        c.request.parse(buffer, readSize);

    if (c.request.parsingCompleted() || c.request.errorOccured()) 
    {
        // parsing completed and we can process the request and work on the response.
    }
}