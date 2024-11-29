#pragma once
#include <cstring>
#include <map>
#include <set>
#include <sys/types.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstddef>
#include <stdexcept>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>  
#include <dirent.h>
#include <ctime>
#include <errno.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>

typedef unsigned char uint8_t;

class HttpMessage {
protected:
    std::map<std::string, std::string> headers;
    std::vector<uint8_t> body; // Raw body data
    std::string version;
    // we can just version "HTTP/1.1" , sinmon  :  400 Bad Request ...

public :
 void resetMessage();
    HttpMessage();
// this after validation of header , can store it in map container
    void addHeader(const std::string& key, const std::string& value);
    std::string getHeader(const std::string& key) const;
    std::string getVersion() const;
   
};
std::string getPWDVariable();
