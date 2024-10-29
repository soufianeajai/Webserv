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
// this after validation of header , can store it in map container
    void addHeader(const std::string& key, const std::string& value);

    std::string getHeader(const std::string& key) const;

    std::string getVersion() const;
 
    virtual std::string toString() const;
    void resetMessage(){
        version.clear();
        body.clear();
        headers.clear();
    }
};
