#include "ServerSetup.hpp"

#define MAX_CLIENTS 10
void ServerSetup(ParsingConfig &Config)
{

	std::vector<std::vector<int> > serverPorts;
	std::vector<std::string> serverHosts;
	std::map<int, Server> Servers = Config.webServer.getServers();
	int Socket; 
	std::vector<std::vector<struct pollfd> > pollDescriptorsByServer;
	for (std::map<int , Server>::iterator it = Servers.begin(); it != Servers.end(); it++)
	{
		std::vector<int> ports = it->second.portGetter();
		std::vector<struct pollfd> serverSockets;
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
			serverSockets.push_back(pfd);
		}
		serverPorts.push_back(ports);
		serverHosts.push_back(it->second.hostGetter());
		pollDescriptorsByServer.push_back(serverSockets);
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
	

	while (1)
	{
		for (size_t index = 0; index < pollDescriptorsByServer.size(); index++)
		{
			std::vector<struct pollfd>& itt = pollDescriptorsByServer[index];
			int pollfds = poll(itt.data(), itt.size(), 1);
			if (pollfds < 0)
			{
				std::cout << "no file found to read" << std::endl;
				exit(1);
			}
			
			for (size_t i = 0; i < itt.size(); i++)
			{
				if (itt[i].revents & (POLLERR | POLLHUP))
				{
					std::cout << "client disconnected\n";
					close(itt[i].fd);
					itt.erase(itt.begin() + i);
					i--;
					exit(1);
				}
			}
			for (size_t i = 0; i < itt.size(); i++)
			{
				if (itt[i].revents & POLLIN)
				{
					int clientSocket = accept(itt[i].fd, NULL , NULL);
					if (clientSocket < 0)
						break;
					struct pollfd ClientPollFd;
					ClientPollFd.events = POLLIN;
					ClientPollFd.fd = clientSocket;
					itt.push_back(ClientPollFd);
					std::cout << "client request:\nHost: " << serverHosts[index] << " Ports: " << serverPorts[index][i] <<
						" Socket: " << clientSocket << std::endl;
					const char* httpResponse = 
					"HTTP/1.1 200 OK\r\n"
					"Content-Length: 13\r\n"
					"\r\n"
					"Connection OK!";
					send(clientSocket, httpResponse, strlen(httpResponse), 0);
				}
			}

		}
	}
	
}