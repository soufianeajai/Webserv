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



std::string HttpMessage::getVersion() const
{
        return version;
}
std::string getPWDVariable()
{
    for (char** current = environ; *current != NULL; ++current)
    {
        if (std::strncmp(*current, "PWD=", 4) == 0)
            return std::string(*current + 4);
    }
    return ""; // Return an empty string if PATH is not found
}
