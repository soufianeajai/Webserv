#pragma once
#include <string>
#include <map>
#include <map>
#include <set>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <cstring>


class HttpMessage {
protected:
    std::map<std::string, std::string> headers;
    std::vector<uint8_t> body; // Raw body data
    std::string version;
    // we can just version "HTTP/1.1" , sinmon  :  400 Bad Request ...

public :
    HttpMessage();
    void resetMessage();
};
