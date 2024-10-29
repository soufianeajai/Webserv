#pragma once
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class Route {
private:
    std::string path;
    std::set<std::string> allowedMethods; // Set for allowed HTTP methods
    bool autoindex = false;
    std::string defaultFile;
    std::string redirect;
    std::string uploadDir;
    std::set<std::string> cgiExtensions;
public:
    Route(const std::string& path) : path(path) {}
    bool matches(const std::string& uri) const;
    bool isMethodAllowed(const std::string& method) const;
};