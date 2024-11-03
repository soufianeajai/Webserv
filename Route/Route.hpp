#pragma once
#include "../HttpMessage/HttpMessage.hpp"



class Route {
private:
    std::string pathLocation; // if pathlocation  == /uploads 
    std::string root;
    std::set<std::string> allowedMethods; // Set for allowed HTTP methods
    bool autoindex;
    std::string defaultFile;
    bool isDir;
    std::set<std::string> cgiExtensions;


    // http redirection : 
    bool IsRedirection;
    std::string NewPathRedirection;
    int statusCodeRedirection;
public:
    void setPath(std::string path);
    std::string getPath();
    void addAllowedMethod(std::string method);
    std::set<std::string> getAllowedMethods();
    void setRoot(std::string root);
    std::string getRoot();
    void setDefaultFile(std::string defaultFile);
    std::string getDefaultFile();
    void setAutoindex(bool autoindex);
    bool getAutoindex();
    void addCgiExtension(std::string cgiExtension);
    std::set<std::string> getCgiExtensions();
    // Route(const std::string& path) : path(path) {}
    // bool matches(const std::string& uri) const;
    // bool isMethodAllowed(const std::string& method) const;
};