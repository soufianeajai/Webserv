#include "Route.hpp"


void Route::setPath(std::string _path) {
    this->pathLocation = _path;
}
std::string Route::getPath() {
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
std::string Route::getRoot() {
    return this->root;
}
void Route::setDefaultFile(std::string _defaultFile) {
    this->defaultFile = _defaultFile;
}
std::string Route::getDefaultFile() {
    return this->defaultFile;
}
void Route::setAutoindex(bool _autoindex) {
    this->autoindex = _autoindex;
}
bool Route::getAutoindex() {
    return this->autoindex;
}
void Route::addCgiExtension(std::string cgiExtension) {
    this->cgiExtensions.insert(cgiExtension);
}
std::set<std::string> Route::getCgiExtensions() {
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
bool Route::getIsRedirection() {
    return this->IsRedirection;
}
void Route::setUploadDir(std::string _uploadDir) {
    this->uploadDir = _uploadDir;
}
std::string Route::getUploadDir() {
    return this->uploadDir;
}
void Route::isDirSetter(bool _isDir) {
    this->isDir = _isDir;
}
bool Route::isDirGetter() {
    return this->isDir;
}
