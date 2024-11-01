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



HttpResponse::HttpResponse(int index_connection, const Server &server):Pages(server.getErrorPages())
{
    // i
    statusCode = server.request.GetCode();
    switch (statusCode)
    {  
        case 400: reasonPhrase = "Bad Request"; break;
        case 403: reasonPhrase = "Forbidden"; break;
        case 404: reasonPhrase = "Not Found"; break;
        case 405: reasonPhrase = "Method Not Allowed"; break;
        case 500: reasonPhrase = "Internal Server Error"; break;
        case 505: reasonPhrase = "HTTP Version Not Supported"; break;
        default: Pages[statusCode] = request.GetUrl(); break;
    }
}

void HttpResponse::buildingHeaders()
{

}
// HttpResponse::HttpResponse(const HttpRequest &request, const Server &server)
// {

//     contractor just for preparation all data to need
//     add also pages default 200 with url (it will be updated every request possible!!)
//     dynamic error from config file else we be default pages
//     Pages[400] = "docs/errorPages/400.html";
//     Pages[403] = "docs/errorPages/403.html";
//     Pages[404] = "docs/errorPages/404.html";
//     Pages[405] = "docs/errorPages/405.html";
//     Pages[500] = "docs/errorPages/500.html";
//     Pages[505] = "docs/errorPages/505.html";
//     statusCode = code;
//     switch (statusCode)
//     {  
//         case 200: reasonPhrase = "OK"; Pages[code] = url; break;
//         case 400: reasonPhrase = "Bad Request"; break;
//         case 403: reasonPhrase = "Forbidden"; break;
//         case 404: reasonPhrase = "Not Found"; break;
//         case 405: reasonPhrase = "Method Not Allowed"; break;
//         case 500: reasonPhrase = "Internal Server Error"; break;
//         case 505: reasonPhrase = "HTTP Version Not Supported"; break;
//         default: reasonPhrase = "Unknown Status"; break;
//     }
//     addHeader("Content-Type", "text/html"); 
//         LoadPage();
//     //addHeader("Connection", "close"); // check just from request header
//     // after we have body now we need set content-length
//     addHeader("Content-Length", intToString(body.size()));
//     addHeader("Date", getCurrentTimeFormatted());
//     addHeader("Server", "WebServ 1337");
// }

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


std::vector<uint8_t> HttpResponse::buildResponseBuffer()
{
    std::vector<uint8_t> response;

    // 1 status line example : HTTP/1.1 200 OK
    std::ostringstream oss;
    oss << version << " " << statusCode << " " << reasonPhrase << "\r\n";
    response.insert(response.end(), oss.str().begin(), oss.str().end());

    //2 APPEND headers !!
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
    {// Clear the buffer
        oss.str(""); 
        oss << it->first << ": " << it->second << "\r\n";
        response.insert(response.end(), oss.str().begin(), oss.str().end());
    }

    response.push_back('\r');
    response.push_back('\n');

    // add body 
    response.insert(response.end(), body.begin(), body.end());
    return response;
}

