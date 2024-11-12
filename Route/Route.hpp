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
    std::string uploadDir;
    std::set<std::string> cgiExtensions;
    bool IsRedirection;
    std::string NewPathRedirection; // must be file not folder
    int statusCodeRedirection;
public:
    void setPath(std::string path);
    std::string getPath() const;
    void addAllowedMethod(std::string method);
    std::set<std::string> getAllowedMethods();
    void setRoot(std::string root);
    std::string getRoot() const;
    void setDefaultFile(std::string defaultFile);
    std::string getDefaultFile() const;
    void setAutoindex(bool autoindex);
    bool getAutoindex();
    void addCgiExtension(std::string cgiExtension);
    std::set<std::string> getCgiExtensions();
    void setRedirectCode(int redirectCode);
    int getRedirectCode();
    void setRedirectnewPath(std::string redirectnewPath);
    std::string getRedirectnewPath();
    void setIsRedirection(bool IsRedirect);
    bool getIsRedirection() const;
    std::string getNewPathRedirection() const;
    // Route(const std::string& path) : path(path) {}
    // bool matches(const std::string& uri) const;
    // bool isMethodAllowed(const std::string& method) const;
    void setUploadDir(std::string uploadDir);
    std::string getUploadDir();
    void isDirSetter(bool isDir);
    bool isDirGetter() const;
};