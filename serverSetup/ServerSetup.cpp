#include "ServerSetup.hpp"


#define MAX_CLIENTS 

void ft_error(std::string err, int fd)
{
    if  (fd != -1)
        close(fd);
    std::cerr << err << std::endl;
    exit (EXIT_FAILURE);
}

int ServerSocketSearch(int epollFd, std::vector<Server>& servers)
{

    for (size_t i = 0; i < servers.size(); ++i)
    {
        if (servers[i].SearchSockets(epollFd) != -1)
            return i;
    }
    return -1;
}

void bindAndListen(int socket, int port, in_addr_t host)
{
    sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = host;
    if (bind(socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
			ft_error("fail to bind local port", socket);
    if (listen(socket, 10) < 0)
			ft_error("fail to listen for connection",-1);
}

void initializeSocketEpoll(int epollInstance, int SocketId, uint32_t event)
{
    struct epoll_event epollfd;
	epollfd.data.fd = SocketId;
    epollfd.events = event;
	if (epoll_ctl(epollInstance, EPOLL_CTL_ADD, SocketId, &epollfd) == -1)
    {
        close(epollInstance);
        if (event == EPOLLIN)
            ft_error("epoll_ctl failed", SocketId);
    }
}

Server getServerSocketCLient(int client,std::vector<Server> &servers)
{
    std::vector<Server>::iterator it; 
    for (it = servers.begin(); it != servers.end(); ++it)
    {
        if(it->hasClient(client))
            break;
    }
    return *it;
}


void ServerSetup(ParsingConfig &Config)
{
    std::vector<int> ServerSocket;
    std::vector<Server> Servers = Config.webServer.getServers();
	int SocketId;
	int epollInstance = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event evenBuffer[1024];
	if (epollInstance < 0)
        ft_error("epoll create1 failed",-1);


	for (std::vector< Server>::iterator it = Servers.begin(); it != Servers.end(); it++)
	{
		std::vector<int> ports = it->portGetter();
		for (size_t i = 0;i < ports.size();i++)
		{
			SocketId = socket(AF_INET, SOCK_STREAM, 0);
			if (SocketId < 0)
			{   
                std::cerr << "The socket not opened for port: " << ports[i] << std::endl;
                continue;
            }
            fcntl(SocketId, F_SETFL, O_NONBLOCK);
            it->addSocket(SocketId);
			int opt = 1;
			if (setsockopt(SocketId, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
                ft_error("Failed to set SO_REUSEADDR",SocketId);
            it->setIpaddress(it->hostGetter());
			bindAndListen(SocketId, ports[i], it->getIpaddress());
			initializeSocketEpoll(epollInstance, SocketId, POLLIN);
            it->serverSocketSetter(ports[i], SocketId);
		}
    }

	while (1)
    {
        int socketServer;
        int epollEventsNumber = epoll_wait(epollInstance, evenBuffer, 1024, -1);
        // if (epollEventsNumber <= 0)
        //     continue;
        for (int index = 0; index < epollEventsNumber; index++)
        {
            Server CurrentServer;
            Connection CurrentConnection;
            if (evenBuffer[index].events & EPOLLIN)
            {
                if (((socketServer = ServerSocketSearch(evenBuffer[index].data.fd, Servers)) != -1))
                {
                    struct sockaddr_in clientAddr;
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    int newClient = accept(evenBuffer[index].data.fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
                    if (newClient < 0)
                        break;

                    Connection connection(newClient, clientAddr, Servers[socketServer].clientMaxBodySizeGetter());
                    initializeSocketEpoll(epollInstance, newClient, EPOLLIN | EPOLLRDHUP | EPOLLHUP);
                    Servers[socketServer].addConnection(newClient,connection);
                }
                else 
                {
                    CurrentServer = getServerSocketCLient(evenBuffer[index].data.fd,Servers);
                    CurrentConnection = CurrentServer.GetConnection(evenBuffer[index].data.fd);
                    CurrentConnection.readIncomingData(CurrentServer.getRoutes());
                    //if (CurrentConnection.getStatus() == GENARATE_RESPONSE) // dosnt work for simple request get for post data txt
                        evenBuffer[index].events |= EPOLLOUT;
                }
            }    
            if (evenBuffer[index].events & (EPOLLOUT))
            {   
                CurrentServer = getServerSocketCLient(evenBuffer[index].data.fd,Servers);
                CurrentConnection = CurrentServer.GetConnection(evenBuffer[index].data.fd);
        //        CurrentConnection.generateResponse(CurrentServer.errorPagesGetter());
                const char* httpResponse = 
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 15\r\n"
                        "\r\n"
                        "Connection batiii2a jidan!";
                send(evenBuffer[index].data.fd, httpResponse, strlen(httpResponse), MSG_NOSIGNAL);
            //    epoll_ctl(epollInstance, EPOLL_CTL_DEL, evenBuffer[index].data.fd, NULL);
            //    std::cout << "connection closed after sending response" << std::endl;
                if (CurrentConnection.getStatus() == DONE)
                   evenBuffer[index].events &= ~EPOLLOUT;
            }
			// EPOLLRHUP The other end of a socket closed or shut down for writing.
            if (evenBuffer[index].events & (EPOLLRDHUP | EPOLLHUP))
            {
                if (epoll_ctl(epollInstance, EPOLL_CTL_DEL, evenBuffer[index].data.fd, NULL) == -1)
                {
                    perror("epoll_ctl failed to remove client");
                }
                // clean the connection of the fd and remove it from the server 
                close(evenBuffer[index].data.fd);
                std::cout << "Client disconnected\n";
                continue;
            }
        }
        
    }
}