#pragma once
#include "../WebServers/Webservers.hpp"
#include "../ParsingConfig/ParsingConfig.hpp"
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




void ServerSetup(ParsingConfig &Config);

