#include "ServerSetup.hpp"

#define MAX_CLIENTS 10
void ServerSetup(ParsingConfig &Config)
{
	std::map<int, Server> Servers = Config.webServer.getServers();
	std::vector<struct pollfd> fds;
	int Socket; 
	for (std::map<int , Server>::iterator it = Servers.begin(); it != Servers.end(); it++)
	{
		std::vector<int> ServerSockets;
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
			if (listen(Socket, 5) < 0)
			{
				std::cout << "fail to listen for connection\n";
				exit(EXIT_FAILURE);
			}
			fcntl(Socket, F_SETFL, O_NONBLOCK);
			ServerSockets.push_back(Socket);
		}
		struct pollfd pfd;
        pfd.fd = Socket;              // The server socket
        pfd.events = POLLIN;        // Monitor for incoming connections
        fds.push_back(pfd);         // Add to fds array
	}
	// struct pollfd fds[MAX_CLIENTS + ServerSockets.size() + 1];
	while (1)
}