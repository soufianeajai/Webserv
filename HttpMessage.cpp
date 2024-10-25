#include "HttpMessage.hpp"

HttpMessage::HttpMessage():version("HTTP/1.1") {}


void HttpMessage::addHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
}


std::string HttpMessage::getHeader(const std::string& key) const {
        std::map<std::string, std::string>::const_iterator it = headers.find(key);
        if (it != headers.end())
            return it->second;
        return "";
}

void HttpMessage::setBody(const std::string& bodyContent) {
        body = bodyContent;
}

std::string HttpMessage::getBody() const {
        return body;
}

std::string HttpMessage::getVersion() const {
        return version;
}
 
std::string HttpMessage::toString() const {
    std::stringstream ss; //  more efficient than using the + operator for string concatenation,its string dynammic
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {
         ss << it->first << ": " << it->second << "\r\n";
    }
    ss << "\r\n" << body;
    return ss.str();
}