#include "ServerSetup.hpp"


#define MAX_CLIENTS 

void ft_error(std::string err, int fd)
{
    if  (fd != -1)
        close(fd);
    std::cerr << "[Error] ... "<<err << std::endl;
    exit (EXIT_FAILURE);
}

time_t current_time()
{
    return static_cast<time_t>(time(NULL));
}


bool check_fd_timeout(time_t last_access_time)
{
    time_t current_time_val = current_time();
    if (current_time_val - last_access_time > TIMEOUT)
        return true;
    return false;
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

struct epoll_event initializeSocketEpoll(int epollInstance, int SocketId, uint32_t event)
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
    return epollfd;
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
void clearConnections(std::vector<Server>& Servers, bool timout){
    for (std::vector< Server>::iterator it = Servers.begin(); it != Servers.end(); it++)
	{
	    std::map<int, Connection*> connections = it->GetCoonections();
        for (std::map<int, Connection*>::iterator conn_it = connections.begin(); conn_it != connections.end(); ++conn_it)
        {
            
            if (timout)
            {
                if(check_fd_timeout(conn_it->second->get_last_access_time()))
                {
                    if(conn_it->second->getResponse().getPid() != -1)
                        kill(conn_it->second->getResponse().getPid() , SIGKILL);
                    it->closeConnection(conn_it->first);
                }
            }
            else
                it->closeConnection(conn_it->first);
        }
    }
}

void ServerSetup(ParsingConfig &Config)
{
    std::vector<int> ServerSocket;
    std::vector<Server> Servers = Config.getServers();
	int SocketId;
	int epollInstance = epoll_create1(EPOLL_CLOEXEC);
    struct epoll_event evenBuffer[1024];
	if (epollInstance < 0){
        ft_error("epoll create1 failed",-1);
    }


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
			int opt = 1;
			if (setsockopt(SocketId, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
                ft_error("Failed to set SO_REUSEADDR",SocketId);
            it->setIpaddress(it->hostGetter());
			bindAndListen(SocketId, ports[i], it->getIpaddress());
			initializeSocketEpoll(epollInstance, SocketId, EPOLLIN);
            it->serverSocketSetter(ports[i], SocketId);
		}
    }
    
	while (1)
    {
        int socketServer;
        int epollEventsNumber = epoll_wait(epollInstance, evenBuffer, 1024, 1);
        if (epollEventsNumber < 0){
            clearConnections(Servers, false);
            ft_error("Epoll wait failed",-1);
        }

        for (int index = 0; index < epollEventsNumber; index++)
        {
            Server CurrentServer;
            Connection *CurrentConnection;
            if (evenBuffer[index].events & EPOLLIN)
            {
                if (((socketServer = ServerSocketSearch(evenBuffer[index].data.fd, Servers)) != -1))
                {
                    struct sockaddr_in clientAddr;
                    socklen_t clientAddrLen = sizeof(clientAddr);
                    int newClient = accept(evenBuffer[index].data.fd, (struct sockaddr*)&clientAddr, &clientAddrLen);
                    if (newClient < 0)
                        break;
                    struct epoll_event epollfd =  initializeSocketEpoll(epollInstance, newClient, EPOLLIN | EPOLLRDHUP | EPOLLHUP);
                    Servers[socketServer].addConnection(newClient,new Connection(newClient, clientAddr, Servers[socketServer].clientMaxBodySizeGetter(), epollfd, evenBuffer[index].data.fd,current_time()));
                }
                else 
                {
                    CurrentServer = getServerSocketCLient(evenBuffer[index].data.fd,Servers);
                    CurrentConnection = CurrentServer.GetConnection(evenBuffer[index].data.fd);
                    CurrentConnection->set_last_access_time(current_time());
                    CurrentConnection->readIncomingData(CurrentServer.getRoutes());
                    if (CurrentConnection->getStatus() == GENARATE_RESPONSE)
                        evenBuffer[index].events |= EPOLLOUT;
                }
            
            }
    
            if (evenBuffer[index].events & (EPOLLOUT))
            {
                CurrentServer = getServerSocketCLient(evenBuffer[index].data.fd,Servers);
                CurrentConnection = CurrentServer.GetConnection(evenBuffer[index].data.fd);
                CurrentConnection->set_last_access_time(current_time());
                CurrentConnection->generateResponse(CurrentServer.errorPagesGetter(), CurrentServer.hostGetter() 
                    ,CurrentServer.GetPort(CurrentConnection->getsocketserver()),CurrentConnection->get_last_access_time());
                    
                if(CurrentConnection->getStatus() == SENDING_RESPONSE)
                {
                    
                    CurrentConnection->getEpollFd().events = EPOLLOUT;
                    if (epoll_ctl(epollInstance, EPOLL_CTL_MOD, evenBuffer[index].data.fd, &CurrentConnection->getEpollFd()) == -1) {
                        std::cerr << "[Error] ... Failed to register for EPOLLOUT: " << strerror(errno) << std::endl;
                        
                    }
                }
                if (CurrentConnection->getStatus() == DONE)
                {
                    if(epoll_ctl(epollInstance, EPOLL_CTL_DEL, evenBuffer[index].data.fd, NULL) == -1)
                        std::cout << "[Error] ... "<<strerror(errno)<<std::endl;
                }
            }
            
        }
        clearConnections(Servers, true);
    }
}