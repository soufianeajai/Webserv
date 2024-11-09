#include "HttpMessage.hpp"

HttpMessage::HttpMessage():version(""){};

void  HttpMessage::resetMessage(){
    version.clear();
    body.clear();
    headers.clear();
}

void HttpMessage::addHeader(const std::string& key, const std::string& value)
{
        headers[key] = value;
}

std::string HttpMessage::getHeader(const std::string& key) const
{
        auto it = headers.find(key);
        if (it != headers.end())
            return it->second;
        return "";
}

std::string HttpMessage::getVersion() const
{
        return version;
}