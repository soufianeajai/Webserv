#pragma once
#include "../WebServers/Webservers.hpp"
#include "../ParsingConfig/ParsingConfig.hpp"
#include "../Connection/Connection.hpp"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctime>
#include <errno.h>
#include <sys/epoll.h>




void ServerSetup(ParsingConfig &Config);

