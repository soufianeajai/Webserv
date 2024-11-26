#include "Route.hpp"

Route::Route():pathLocation(""), root(""), autoindex(false), defaultFile(""), uploadDir(""), IsRedirection(false), NewPathRedirection(""){}
int Route::getstatusCodeRedirection() const
{
    return statusCodeRedirection;
}

void Route::setPath(std::string _path) {
    this->pathLocation = _path;
}
std::string Route::getPath() const{
    return this->pathLocation;
}
void Route::addAllowedMethod(std::string method) {
    this->allowedMethods.insert(method);
}
std::set<std::string> Route::getAllowedMethods() {
    return this->allowedMethods;
}
void Route::setRoot(std::string _root) {
    this->root = _root;
}
std::string Route::getRoot() const
{
    return root;
}
void Route::setDefaultFile(std::string _defaultFile) {
    this->defaultFile = _defaultFile;
}
std::string Route::getDefaultFile() const{
    return defaultFile;
}
void Route::setAutoindex(bool _autoindex) {
    this->autoindex = _autoindex;
}
bool Route::getAutoindex() {
    return this->autoindex;
}
void Route::addCgiExtension(std::string cgiExtension, std::string cgiPath) {
    this->cgiExtensions[cgiExtension] = cgiPath;
}
std::map<std::string, std::string> Route::getCgiExtensions() {
    return this->cgiExtensions;
}
void Route::setRedirectnewPath(std::string _redirectnewPath) {
    this->NewPathRedirection = _redirectnewPath;
}
std::string Route::getRedirectnewPath() {
    return this->NewPathRedirection;
}

void Route::setRedirectCode(int _redirectCode) {
    this->statusCodeRedirection = _redirectCode;
}
int Route::getRedirectCode() {
    return this->statusCodeRedirection;
}
void Route::setIsRedirection(bool IsRedirect) {
    this->IsRedirection = IsRedirect;
}
bool Route::getIsRedirection() const{
    return this->IsRedirection;
}
void Route::setUploadDir(std::string _uploadDir) {
    this->uploadDir = _uploadDir;
}
std::string Route::getUploadDir() {
    return this->uploadDir;
}

std::string Route::getNewPathRedirection() const
{
    return NewPathRedirection;
}