#include "ServerSetup.hpp"
#include <signal.h>
// #include <csignal> 

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
		// fdsServer[it->second.hostGetter()] = pollDescriptorsByServer;
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
	
	size_t lastServerSocket = pollDescriptorsByServer.size();
	while (1)
	{
		signal(SIGPIPE, SIG_IGN);
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
				if (pollDescriptorsByServer[index].revents & POLLIN)
				{
					int clientSocket = accept(pollDescriptorsByServer[index].fd, NULL , NULL);
					if (clientSocket < 0)
						break;
					struct pollfd ClientPollFd;
					ClientPollFd.events = POLLIN;
					ClientPollFd.fd = clientSocket;
					pollDescriptorsByServer.push_back(ClientPollFd);
					std::cout << "new connection\n";
					const char* httpResponse = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Length: 13\r\n"
					"\r\n"
					"Connection OK!";
					send(pollDescriptorsByServer.back().fd, httpResponse, strlen(httpResponse), 0);
				}
			}
			else
			{
				char buffer[1024];
				int bytesRead = recv(pollDescriptorsByServer[index].fd, buffer, sizeof(buffer), 0);
				if (bytesRead > 0)
				{
					std::cout << "Request received: " << buffer << std::endl;
				}
				else if (bytesRead == 0)
				{
					close(pollDescriptorsByServer[index].fd);
					pollDescriptorsByServer.erase(pollDescriptorsByServer.begin() + index);
					std::cout << "Client disconnected\n";
				}
				memset(buffer, 0, sizeof(buffer));
				const char* httpResponse = 
				"HTTP/1.1 200 OK\r\n"
				"Content-Length: 13\r\n"
				"\r\n"
				"Connection OK!";
				send(pollDescriptorsByServer.back().fd, httpResponse, strlen(httpResponse), 0);
			}
		}
	}
}