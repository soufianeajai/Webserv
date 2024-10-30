#include "HttpResponse.hpp"


//1xx (Informational): The request was received, continuing process

//2xx (Successful): The request was successfully received,
//understood, and accepted

//3xx (Redirection): Further action needs to be taken in order to
//complete the request

//4xx (Client Error): The request contains bad syntax or cannot be
//fulfilled

//5xx (Server Error): The server failed to fulfill an apparently
//valid request

std::string intToString(size_t number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}


std::string getCurrentTimeFormatted()
{
    time_t rawTime;
    struct tm *timeInfo;
    char buffer[80];
    // Get current time
    time(&rawTime);
    timeInfo = gmtime(&rawTime); // Use gmtime for GMT time
    // Format the date and time as "Wed, 29 Oct 2024 15:30:00 GMT"
    strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
    return std::string(buffer);
}


HttpResponse::HttpResponse(int code, std::string url)
{
    //contractor just for preparation all data to need
    // add also pages default 200 with url (it will be updated every request possible!!)
    if (code < 400)
        Pages[code] = url;
    // dynamic error from config file
    Pages[400] = "docs/errorPages/400.html";
    Pages[403] = "docs/errorPages/403.html";
    Pages[404] = "docs/errorPages/404.html";
    Pages[405] = "docs/errorPages/405.html";
    Pages[500] = "docs/errorPages/500.html";
    Pages[505] = "docs/errorPages/505.html";
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

void HttpResponse::GenerateResponse(const Request& request)
{
    addHeader("Content-Type", "text/html");
    
        //addHeader("Connection", "close"); // signall from listen socket fd for client 
    // after we have body now we need set content-length
    addHeader("Content-Length", intToString(body.size()));
    addHeader("Date", getCurrentTimeFormatted());
    addHeader("Server", "WebServ 1337");
    
}

void HttpResponse::LoadPage()
{
    
    std::map<int, std::string>::iterator it = Pages.find(statusCode);
    if (it != Pages.end())
    {
        const std::string& fileName = it->second;
        std::ifstream file(fileName.c_str());
        if (file.is_open())
            body.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
}