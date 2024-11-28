#include "HttpRequest.hpp"

void HttpRequest::handleTransfer(){
    if (headers.find("Transfer-Encoding") != headers.end() && headers["Transfer-Encoding"] == "chunked"){
        isChunked = true;
    }
    if (headers.find("Content-Length") != headers.end()){
        isContentLength = true;
        char c = 0;
        long long temp;
        std::istringstream nbr(headers["Content-Length"]);
        nbr >> temp >> c;
        contentLength =  (temp <= 0 || c) ? 0 : temp;
    }
    if (headers.find("Content-Type") != headers.end() && isValidMultipart(headers["Content-Type"])){
        isMultipart = true;
    }
}

bool HttpRequest::isValidMultipart(std::string content) {
    std::size_t boundaryPos = content.find("boundary=");
    if (boundaryPos != std::string::npos && boundaryPos == content.find("multipart/form-data; ") + 21)
    {
        boundary = content.substr(boundaryPos + 9);
        return true;
    }
    else
        boundary.clear();
    return false;
}

bool HttpRequest::isValidHeaderNameChar(uint8_t byte) {
    return std::isalnum(byte) || byte == '-' || byte == '_';
}


void HttpRequest::addCurrentHeader() {
    if (!currentHeaderName.empty()) {
        size_t start = currentHeaderValue.find_first_not_of(" \t");
        size_t end = currentHeaderValue.find_last_not_of(" \t");
        if (start != std::string::npos && end != std::string::npos) {
            currentHeaderValue = currentHeaderValue.substr(start, end - start + 1);
        }
        headers[currentHeaderName] = currentHeaderValue;
        currentHeaderName.clear();
        currentHeaderValue.clear();
    }
}

bool HttpRequest::errorOccured() const {
    for (std::map<State, int>::const_iterator it = errorState.begin(); it != errorState.end(); it++){
        if (it->first == currentState)
            return true;
    }
    return false;
}


bool HttpRequest::isValidPathChar(uint8_t byte) {
    return (byte >= 33 && byte <= 126) &&
        byte != ' ' &&
        byte != '<' && byte != '>' &&
        byte != '[' && byte != ']' &&
        byte != '{' && byte != '}' &&
        byte != '"' && byte != '\\' &&
        byte != '^' && byte != '`';      
    }
bool HttpRequest::uriBehindRoot() {
    std::istringstream stream(uri);
    std::string segment;
    int pos = 0;

    while (std::getline(stream, segment, '/'))
    {
        if (segment == "..")
            pos--;
        else if (!segment.empty() && segment != ".")
            pos++;
    }
    return (pos < 0);
}
