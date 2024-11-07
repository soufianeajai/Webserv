#include "ServerSetup.hpp"
#include <signal.h>
#include <ctime>
#include <errno.h>
#include <sys/epoll.h>

#define MAX_CLIENTS 
bool ServerSocketSearch(int epollFds, std::vector<int> ServerSockets)
{
	for (size_t i = 0; i < ServerSockets.size(); i++)
		if (epollFds == ServerSockets[i])
			return true;
	return false;	
}
void ServerSetup(ParsingConfig &Config)
{
	int epollInstance = epoll_create1(EPOLL_CLOEXEC);
	if (epollInstance < 0)
	{
		std::cout << "epoll create1 failed\n";
		exit(1);
	}
	std::map<int, Server> Servers = Config.webServer.getServers();
	int Socket; 

	// std::vector<struct pollfd> pollDescriptorsByServer;
	std::vector<int> ServersSocket;
	for (std::map<int , Server>::iterator it = Servers.begin(); it != Servers.end(); it++)
	{
		std::vector<int> ports = it->second.portGetter();
		for (size_t i = 0;i < ports.size();i++)
		{
			Socket = socket(AF_INET, SOCK_STREAM, 0);
			if (Socket < 0)
				std::cout << "The socket not opened\n";
			int opt = 1;
			if (setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
				std::cerr << "Failed to set SO_REUSEADDR\n";
				close(Socket);
				exit(EXIT_FAILURE);
			}
			sockaddr_in serverAddress;
			serverAddress.sin_family = AF_INET;
    		serverAddress.sin_port = htons(ports[i]);
    		serverAddress.sin_addr.s_addr = inet_addr(it->second.hostGetter().c_str());

			if (bind(Socket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
			{
				std::cout << "fail to bind local port " << it->second.hostGetter() << " " << ports[i] << std::endl;
				exit(EXIT_FAILURE);
			}
			if (listen(Socket, 10) < 0)
			{
				std::cout << "fail to listen for connection\n";
				exit(EXIT_FAILURE);
			}
			fcntl(Socket, F_SETFL, O_NONBLOCK);


			struct epoll_event epollfd;
			epollfd.data.fd = Socket;
			epollfd.events = EPOLLIN;
			if (epoll_ctl(epollInstance, EPOLL_CTL_ADD, Socket, &epollfd) == -1)
			{
				perror("epoll_ctl failed");
        		close(epollInstance);
        		exit(EXIT_FAILURE);
			}
			ServersSocket.push_back(Socket);
		}
	}

	struct epoll_event evenBuffer[1024];
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
            if (ServerSocketSearch(evenBuffer[index].data.fd, ServersSocket))
            {
                int newClient = accept(evenBuffer[index].data.fd, NULL, NULL);
                if (newClient < 0)
                    break;
                fcntl(newClient, F_SETFL, O_NONBLOCK);

                struct epoll_event clientEpollFd;
                clientEpollFd.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
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
                    
                    const char* httpResponse = 
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Length: 13\r\n"
                        "\r\n"
                        "Connection OK!";
                    send(newClient, httpResponse, strlen(httpResponse), MSG_NOSIGNAL);
                    
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
                        
                        const char* httpResponse = 
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Length: 13\r\n"
                            "\r\n"
                            "Connection OK!";
                        send(evenBuffer[index].data.fd, httpResponse, strlen(httpResponse), MSG_NOSIGNAL);
                        
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
                }
            }
        }
    }
}