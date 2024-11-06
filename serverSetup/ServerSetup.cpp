#include "ServerSetup.hpp"
       #include <signal.h>

#define MAX_CLIENTS 10

void ServerSetup(ParsingConfig &Config)
{
	int Socket;
	std::map<int, Server> Servers = Config.webServer.getServers();
	std::map<int, int> ports;
	std::map<int , std::string> hosts;
	std::vector<struct pollfd> pollDescriptorsByServer;
	int PortIndexing = 0;
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
			ports[PortIndexing++] = ports[i];
		}
	}
	// for (size_t i = 0; i < pollDescriptorsByServer.size() && i < serverPorts.size(); i++)
	// {
	// 	std::vector<struct pollfd> itt = pollDescriptorsByServer[i];
	// 	std::vector<int> ports = serverPorts[i];
	// 	for (size_t j = 0; j < itt.size(); j++)
	// 	{
	// 		std::cout << ports[i] << ": " <<  itt[j].fd << " | ";
	// 	}
	// 	std::cout << std::endl;
	// }
	std::vector<struct sockaddr_in> addrArray;
	size_t lastServerSocket = pollDescriptorsByServer.size();
	while (1)
	{
		signal(SIGPIPE, SIG_IGN);
		
		for (size_t index = 0; index < pollDescriptorsByServer.size(); index++)
		{
			// std::cout << "break :" << index << std::endl;
			struct sockaddr_in addr;
			socklen_t addrlen = sizeof(addr);
			// if (pollDescriptorsByServer.size() + 1 == MAX_CLIENTS)
			// {
			// 		std::cout << "the server has readched its max\n";
			// 	exit(1);
			// }
			int pollfds = poll(pollDescriptorsByServer.data(), pollDescriptorsByServer.size(), 1);
			if (pollfds < 0)
			{
				std::cout << "no file found to read" << std::endl;
				// exit(1);
			}
			
			for (size_t i = 0; i < pollDescriptorsByServer.size(); i++)
			{

				if (pollDescriptorsByServer[i].revents & POLLIN)
				{
					if (i < lastServerSocket) 
					{
						int clientSocket = accept(pollDescriptorsByServer[i].fd, (struct sockaddr *)&addr , &addrlen);
						if (clientSocket < 0)
						{
							// std::cout << "break";
							break;
						}
						std::cout << "Client trying to connect: "
								<< inet_ntoa(addr.sin_addr) << ":" << ntohl(addr.sin_port) << std::endl;
						// This is a new connection
						struct pollfd ClientPollFd;
						ClientPollFd.events = POLLIN;
						ClientPollFd.fd = clientSocket;
						pollDescriptorsByServer.push_back(ClientPollFd);
																	const char* httpResponse = 
						"HTTP/1.1 200 OK\r\n"
						"Content-Length: 14\r\n"
						"\r\n"
						"Connection OK!\n";
						send(pollDescriptorsByServer.back().fd, httpResponse, strlen(httpResponse), 0);

					}
					else{
					const char* httpResponse2 = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Length: 15\r\n"
					"\r\n"
					"Connection OKK!\n";

				
					send(pollDescriptorsByServer[i].fd, httpResponse2, strlen(httpResponse2), 0);
					}
				}
				// if (pollDescriptorsByServer[i].revents & POLLOUT)
				// {
				// 	if (i < lastServerSocket) 
				// 	{
				// 							const char* httpResponse = 
				// 		"HTTP/1.1 200 OK\r\n"
				// 		"Content-Length: 14\r\n"
				// 		"\r\n"
				// 		"Connection OK!\n";
				// 		send(pollDescriptorsByServer.back().fd, httpResponse, strlen(httpResponse), 0);
				// 	}
				// 	else{
				// 	const char* httpResponse2 = 
				// 	"HTTP/1.1 200 OK\r\n"
				// 	"Content-Length: 15\r\n"
				// 	"\r\n"
				// 	"Connection OKK!\n";

				
				// 	send(pollDescriptorsByServer[i].fd, httpResponse2, strlen(httpResponse2), 0);
				// 	}
				// }
			}
		}
	}
	
}