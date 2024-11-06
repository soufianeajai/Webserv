#include "ServerSetup.hpp"
#include <signal.h>
#include <ctime>
#include <errno.h>

#define MAX_CLIENTS 10

void ServerSetup(ParsingConfig &Config)
{
	std::map<int, Server> Servers = Config.webServer.getServers();
	int Socket; 
	std::vector<struct pollfd> pollDescriptorsByServer;
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

			struct pollfd pfd;
			pfd.fd = Socket;              
			pfd.events = POLLIN;           
			pollDescriptorsByServer.push_back(pfd);
		}
	}

	size_t lastServerSocket = pollDescriptorsByServer.size();
	// time_t time;
	while (1)
	{
		int pollfds = poll(pollDescriptorsByServer.data(), pollDescriptorsByServer.size(), 1);
		if (pollfds < 0)
		{
			std::cout << "no file found to read" << std::endl;
			exit(1);
		}
		for (size_t index = 0; index < pollDescriptorsByServer.size(); index++)
		{
			if (index < lastServerSocket)
			{
				// hna kantchekiw wach server tra fih chi event in case tra event khassna acceptiw new connection
				// 
				if (pollDescriptorsByServer[index].revents & POLLIN)
				{
					int clientSocket = accept(pollDescriptorsByServer[index].fd, NULL , NULL);
					if (clientSocket < 0)
						break;
					std::cout << "new conection\n";
					
					struct pollfd ClientPollFd;
					// means that we are interested in reading from this client
					ClientPollFd.events = POLLIN;
					ClientPollFd.fd = clientSocket;

					pollDescriptorsByServer.push_back(ClientPollFd);

					char buffer[1024];
					/*
						flags:
							MSG_OOB:For retrieving urgent data sent with send(MSG_OOB)
							MSG_PEEK:For peeking at the incoming data without removing it from the input queue
							MSG_WAITALL:For blocking until the full amount of data is received
							0: regular behavior
					*/
					int bytesRead = recv(pollDescriptorsByServer.back().fd, buffer, sizeof(buffer), 0);
					if (bytesRead > 0)
					{
						std::cout << "Request received from new connection: " << buffer << std::endl;
					}
					else if (bytesRead == 0)
					{
						close(pollDescriptorsByServer[index].fd);
						pollDescriptorsByServer.erase(pollDescriptorsByServer.begin() + index);
						std::cout << "Client disconnected\n";
					}
					memset(buffer, 0, sizeof(buffer));
					// so that we can send a response to the client
					pollDescriptorsByServer.back().revents = POLLOUT;
				}
				if (pollDescriptorsByServer.back().revents & POLLOUT)
				{
					const char* httpResponse = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Length: 13\r\n"
					"\r\n"
					"Connection OK!";
					send(pollDescriptorsByServer.back().fd, httpResponse, strlen(httpResponse), MSG_NOSIGNAL);
					// set revenets to POLLIN because i want to keep reading from the client
					pollDescriptorsByServer.back().revents = POLLIN;
				}
			}
			else
			{
				// had l condition katchki ala kola client wach fih chi 7aja mat reada in case makanch walo
				// ghadi checkiha later 7itach socket non-blocking
				if (pollDescriptorsByServer[index].revents & POLLIN)
				{
					char buffer[1024];
					int bytesRead = recv(pollDescriptorsByServer[index].fd, buffer, sizeof(buffer), 0);
					if (bytesRead > 0)
					{
						std::cout << "Request received from existing connection: " << buffer << std::endl;
					}
					else if (bytesRead == 0)
					{
						close(pollDescriptorsByServer[index].fd);
						pollDescriptorsByServer.erase(pollDescriptorsByServer.begin() + index);
						std::cout << "Client disconnected\n";
					}
					memset(buffer, 0, sizeof(buffer)); // bach nfreei lbuffer
					pollDescriptorsByServer[index].revents = POLLOUT;
				}
				if (pollDescriptorsByServer[index].revents & POLLOUT)
				{
					const char* httpResponse = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Length: 13\r\n"
					"\r\n"
					"Connection OK!";
					send(pollDescriptorsByServer[index].fd, httpResponse, strlen(httpResponse), MSG_NOSIGNAL);
					pollDescriptorsByServer[index].revents = POLLIN;
				}
			}
		}
	}
}