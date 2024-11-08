#include "ServerSetup.hpp"
#include <signal.h>
#include <ctime>
#include <errno.h>
#include <sys/epoll.h>

#define MAX_CLIENTS 

void ft_error(std::string err, int fd)
{
    if  (fd != -1)
        close(fd);
    std::cerr << err << std::endl;
    exit (EXIT_FAILURE);
}

bool ServerSocketSearch(int epollFd, std::vector<int> ServersSocketsId)
{
    for (size_t i = 0; i < ServersSocketsId.size(); ++i)
    {
        if (ServersSocketsId[i] == epollFd) 
            return true;
    }
    return false;
}

void bindAndListen(int socket, int port, const char* host)
{
    sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(host);
    if (bind(socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
			ft_error("fail to bind local port", socket);
    if (listen(socket, 10) < 0)
			ft_error("fail to listen for connection",-1);
    fcntl(socket, F_SETFL, O_NONBLOCK);
}

void initializeSocketEpoll(int epollInstance, int SocketId)
{
    struct epoll_event epollfd;
	epollfd.data.fd = SocketId;
    epollfd.events = EPOLLIN;
	if (epoll_ctl(epollInstance, EPOLL_CTL_ADD, SocketId, &epollfd) == -1)
    {
        close(epollInstance);
        ft_error("epoll_ctl failed", SocketId);
    }
}

void ServerSetup(ParsingConfig &Config)
{
    std::vector<Server> Servers = Config.webServer.getServers();
	int SocketId;
	int epollInstance = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event evenBuffer[1024];

	if (epollInstance < 0)
        ft_error("epoll create1 failed",-1);
    std::vector<int> ServerSocket;


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
            ServerSocket.push_back(SocketId);
			int opt = 1;
			if (setsockopt(SocketId, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
                ft_error("Failed to set SO_REUSEADDR",SocketId);
			bindAndListen(SocketId,ports[i], it->hostGetter().c_str());
			initializeSocketEpoll(epollInstance, SocketId);
            it->serverSocketSetter(ports[i], SocketId);
		}
    }

	while (1)
    {
        int epollEventsNumber = epoll_wait(epollInstance, evenBuffer, 1024, 1);
        if (epollEventsNumber < 0)
        {
            std::cout << "an error occured\n";
            continue;
        }
        for (int index = 0; index < epollEventsNumber; index++)
        {
            if (ServerSocketSearch(evenBuffer[index].data.fd, ServerSocket))
            {
                // Server sev = Servers[evenBuffer[index].data.fd];
                // Connection *cnx;
                int newClient = accept(evenBuffer[index].data.fd, NULL, NULL);

                if (newClient < 0)
                    break;
                fcntl(newClient, F_SETFL, O_NONBLOCK);

                struct epoll_event clientEpollFd;
                clientEpollFd.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLHUP;
                clientEpollFd.data.fd = newClient;

                if (epoll_ctl(epollInstance, EPOLL_CTL_ADD, newClient, &clientEpollFd) == -1)
                {
                    perror("epoll_ctl failed 1");
                    close(newClient);
                    continue;
                }

                char buffer[1024];
				
                int bytesRead = recv(newClient, buffer, sizeof(buffer), 0);
                if (bytesRead > 0)
                {
                    std::cout << "Request received from new connection: " << buffer << std::endl;
                }
                else if (bytesRead == 0)
                {
					std::cout << bytesRead << std::endl;
                    if (epoll_ctl(epollInstance, EPOLL_CTL_DEL, newClient, NULL) == -1)
                    {
                        perror("epoll_ctl failed 2");
                    }
                    close(newClient);
                }
                memset(buffer, 0, sizeof(buffer));


                // send a response
                if (clientEpollFd.events & EPOLLOUT)
                {
                    const char* httpResponse = 
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 13\r\n"
                        "\r\n"
                        "Connection OK!";
                    send(newClient, httpResponse, strlen(httpResponse), MSG_NOSIGNAL);
                }
            }
            else
            {
				// EPOLLRHUP The other end of a socket closed or shut down for writing.
                if (evenBuffer[index].events & (EPOLLRDHUP | EPOLLHUP))
                {
                    if (epoll_ctl(epollInstance, EPOLL_CTL_DEL, evenBuffer[index].data.fd, NULL) == -1)
                    {
                        perror("epoll_ctl failed to remove client");
                    }
                    close(evenBuffer[index].data.fd);
                    std::cout << "Client disconnected\n";
                    continue;
                }

                if (evenBuffer[index].events & EPOLLIN)
                {
                    char buffer[1024];
                    int bytesRead = recv(evenBuffer[index].data.fd, buffer, sizeof(buffer), 0);
                    if (bytesRead > 0)
                    {
                        std::cout << "Request received from existing connection: " << buffer << std::endl;  
                    }
                    else if (bytesRead == 0)
                    {
                        if (epoll_ctl(epollInstance, EPOLL_CTL_DEL, evenBuffer[index].data.fd, NULL) == -1)
                        {
                            perror("epoll_ctl failed to remove client");
                        }
                        close(evenBuffer[index].data.fd);
                        std::cout << "Client disconnected\n";
                    }
                    memset(buffer, 0, sizeof(buffer));
                    // send a response
                    if (evenBuffer[index].events & EPOLLOUT)
                    {
                        const char* httpResponse = 
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Length: 13\r\n"
                            "\r\n"
                            "Connection OK!";
                        send(evenBuffer[index].data.fd, httpResponse, strlen(httpResponse), MSG_NOSIGNAL);
                    }
                }
            }
        }
    }
}