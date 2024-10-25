#include "HttpResponse.hpp"

//external  functions :
std::string intToString(int number) {
    std::stringstream ss;
    ss << number;
    return ss.str();
}


HttpResponse::HttpResponse():statusCode(200),reasonPhrase("OK")
{
        errorPages[400] = "docs/errorPages/400.html";
        errorPages[403] = "docs/errorPages/403.html";
        errorPages[404] = "docs/errorPages/404.html";
        errorPages[405] = "docs/errorPages/405.html";
        errorPages[500] = "docs/errorPages/500.html";
        errorPages[505] = "docs/errorPages/505.html";
}
void HttpResponse::setStatusCode(int code)
{
    statusCode = code;
    switch (statusCode)
    {  
        case 200: reasonPhrase = "OK"; break;
        case 400: reasonPhrase = "Bad Request"; break;
        case 403: reasonPhrase = "Forbidden"; break;
        case 404: reasonPhrase = "Not Found"; break;
        case 405: reasonPhrase = "Method Not Allowed"; break;
        case 500: reasonPhrase = "Internal Server Error"; break;
        case 505: reasonPhrase = "HTTP Version Not Supported"; break;
        default: reasonPhrase = "Unknown Status"; break;
    }
}
std::string HttpResponse::getReasonPhrase() const
{
        return reasonPhrase;
}

std::string HttpResponse::toString() const
{
        std::stringstream ss;
        ss << version << " " << statusCode << " " << reasonPhrase << "\r\n";
        ss << HttpMessage::toString();
        return ss.str();
}

void  HttpResponse::LoadErrorPage()
{
    std::map<int, std::string>::iterator it = errorPages.find(statusCode);
    if (it != errorPages.end())
    {
        const std::string& fileName = it->second;
        std::ifstream file(fileName.c_str());
        if (file.is_open())
        {
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            addHeader("Content-Type", "text/html");
            addHeader("Content-Length", intToString(buffer.str().size()));
            setBody(buffer.str());
        }
    }
}
