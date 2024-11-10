#include "HttpResponse.hpp"
#include <sys/stat.h>

HttpResponse::HttpResponse(){}

bool isDirectory(const std::string& path)
{
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) != 0)
    {
        perror("stat");  // Print an error if stat fails
        return false;    // Treat as non-directory if there's an error
    }
    return S_ISDIR(pathStat.st_mode);  // Check if it's a directory
}

std::string createSetCookieHeader(const std::string& sessionId)
{
        return "session_id=" + sessionId + "; Path=/; HttpOnly";
}

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

//1xx (Informational): The request was received, continuing process

//2xx (Successful): The request was successfully received,
//understood, and accepted

//3xx (Redirection): Further action needs to be taken in order to
//complete the request

//4xx (Client Error): The request contains bad syntax or cannot be
//fulfilled

//5xx (Server Error): The server failed to fulfill an apparently
//valid request

void HttpResponse::initResponse(const Route &route,std::string errorPage, int code,const std::string &query, const std::string UrlRequest, const std::string method)
{
    statusCode = code;
    this->query = query;
    switch (statusCode)
    {  
        case 300: reasonPhrase = "Multiple Choices"; break;
        case 301: reasonPhrase = "Moved Permanently"; break;
        case 302: reasonPhrase = "Found"; break;  // Also called "Temporary Redirect" in some contexts
        case 303: reasonPhrase = "See Other"; break;
        case 304: reasonPhrase = "Not Modified"; break;
        case 307: reasonPhrase = "Temporary Redirect"; break;
        case 308: reasonPhrase = "Permanent Redirect"; break;
        case 400: reasonPhrase = "Bad Request"; break;
        case 403: reasonPhrase = "Forbidden"; break;
        case 404: reasonPhrase = "Not Found"; break;
        case 405: reasonPhrase = "Method Not Allowed"; break;
        case 500: reasonPhrase = "Internal Server Error"; break;
        case 505: reasonPhrase = "HTTP Version Not Supported"; break;
        default:  reasonPhrase = "OK"; break; // ???
    }

    if (statusCode > 399)
        Page = errorPage;
    else if(route.getIsRedirection())
        Page = route.getRoot() + route.getNewPathRedirection();
    else
        Page = route.getRoot() +  UrlRequest;


    if (isDirectory(Page))
    {
        // default file or auto index 
    }
    else
    {
        // is good is file so fitch content of file 

    }

    // MIME !!

    std::cout << "\n___________________________________:"<<UrlRequest<<":___________________________________\n";
    mimeTypes["html"] = "text/html";
    mimeTypes["css"] = "text/css";
    mimeTypes["js"] = "application/javascript";
    mimeTypes["json"] = "application/json";
    mimeTypes["xml"] = "application/xml";
    mimeTypes["jpg"] = "image/jpeg";
    mimeTypes["jpeg"] = "image/jpeg";
    mimeTypes["png"] = "image/png";
    mimeTypes["gif"] = "image/gif";
    mimeTypes["svg"] = "image/svg+xml";
    mimeTypes["txt"] = "text/plain";
    mimeTypes["pdf"] = "application/pdf";
    mimeTypes["zip"] = "application/zip";
    mimeTypes["mp3"] = "audio/mpeg";
    mimeTypes["mp4"] = "video/mp4";
}

/* Find the position of the '?' and pos of / to get script name
        /cgi/script.php/test/more/path?param1=value1&param2=value2
                                     pos + 1 = p
            queryString =   param1=value1&param2=value2
            scriptName =    /cgi/script.php
            path_info = /test/more/path                
    */
size_t HttpResponse::checkIfCGI(const std::string& url)
{
    size_t scriptEndPos = std::string::npos;

    // Iterate over the set of valid CGI extensions
    for (std::set<std::string>::const_iterator it = ValidcgiExtensions.begin(); it != ValidcgiExtensions.end(); ++it)
    {
        const std::string& ext = *it;
        size_t pos = url.rfind(ext);
//extension is not at the end or followed by a '/' its invalid like : /cgi/somefile.php.invalid
        if (pos != std::string::npos &&
            (pos + ext.length() == url.length() || url[pos + ext.length()] == '/'))
        {
            scriptEndPos = pos + ext.length();
            break; 
        }
    }

    return scriptEndPos;
}


// std::vector<char*> HttpResponse::createEnvChar(const Connection& connection, size_t scriptEndPos) const
// {
//     std::vector<char*> envVars;
//     std::string url = connection.getRequest().getUrl();

//     std::string scriptName;
//     std::string pathInfo;
//     std::string queryString;
//     std::string contentType;
//     queryString = connection.getRequest().GetQuery();
//     scriptName = url.substr(0, scriptEndPos);
//     pathInfo =  url.substr(scriptEndPos);
//             = (connection.getRequest().getMethod() == "POST") ? connection.getRequest().getHeader("CONTENT_TYPE") : "";

//     env.push_back(const_cast<char*>(("REQUEST_METHOD=" + connection.getRequest().getMethod()).c_str()));
//     env.push_back(const_cast<char*>(("QUERY_STRING=" + queryString).c_str()));
//     env.push_back(const_cast<char*>(("SCRIPT_NAME=" + scriptName).c_str()));
//     env.push_back(const_cast<char*>(("PATH_INFO=" + pathInfo).c_str()));
//     env.push_back(const_cast<char*>(("CONTENT_TYPE=" + contentType).c_str()));
//     env.push_back(const_cast<char*>("GATEWAY_INTERFACE=CGI/1.1"));
//     env.push_back(const_cast<char*>("SERVER_PROTOCOL=" + version).c_str());
//     //REMOTE_ADDR,REMOTE_HOST,SERVER_NAME,SERVER_PORT,AUTH_TYPE
//     return envVars;
// }






// void HttpResponse::executeCGI(const std::string& scriptPath,std::vector<char*> &envp)
// {
    
//     int pipefd[2];
//     if (pipe(pipefd) == -1) {
//         perror("pipe");
//         return;
//     }
//     pid_t pid = fork();
//     if (pid == -1) {
//         perror("fork");
//         return;
//     }
//     if (pid == 0)
//     {
//         // In child process
//         dup2(pipefd[1], STDOUT_FILENO);
//         close(pipefd[0]);
//         close(pipefd[1]); 
//         char* argv[] = {const_cast<char*>(scriptPath.c_str()), NULL};
//         execve(scriptPath.c_str(), argv, &envp[0]); // envp.data() but is in c++11
//     }
//     else
//     {
//         // In parent process
//         close(pipefd[1]);
//         std::string cgiOutput;
//         char buffer[1024];
//         ssize_t bytesRead;
//          while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
//             cgiOutput.append(buffer, buffer + bytesRead);
//         close(pipefd[0]);
//         waitpid(pid, NULL, 0);
//         body.assign(cgiOutput.begin(), cgiOutput.end());
//     }
// }





void HttpResponse::LoadPage()
{
    std::ifstream file(Page.c_str());
    if (file.is_open()) // always is true 
        body.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

// // set best type of mime for content-type !!
// std::string HttpResponse::getMimeType(const std::string& filePath) const
// {
//     size_t pos = filePath.find_last_of('.');
//     if (pos == std::string::npos)
//         return "application/octet-stream";
//     std::string extension = filePath.substr(pos + 1);
//     std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
//     if (it != mimeTypes.end())
//         return it->second;
//     return "application/octet-stream";  // Default type if extension not found
// }

// void HttpResponse::generateResponse(const Connection &connection)
// {
//     // detect CGI  
//     // url /cgi/script.php
//     bool flagConnection ; 
//     std::string url = connection.getRequest().getUrl();
//     size_t scriptEndPos = checkIfCGI(url);  
//     if (scriptEndPos !=  std::string::npos)
//         executeCGI(createEnvMap(connection, scriptEndPos));  // createEnvMap builds necessary env variables
//     else
//         LoadPage();

//     addHeader("Content-Type", getMimeType(url); // content-type for cgi ?
    
//     //Transfer-Encoding: chunked or content-length ?
//     addHeader("Content-Length", intToString(body.size()));
//     addHeader("Date", getCurrentTimeFormatted());
//     addHeader("Server", "WebServ 1337");  
//     addHeader("Connection",connection.GetflagConnection() ? "keep-alive" : "close");

//     //Location: https://example.com/new-path
//     // from config file  : redirect: old-path 3xx new-path
//     //The browser automatically makes a new HTTP request to the URL specified in the Location header
//     if (statusCode > 299 && statusCode < 400)
//         addHeader("Location", "localhost?" + "new-path");

//     // cookies header ?
//     // addHeader("Set-Cookie",createSetCookieHeader());
// }


// std::vector<uint8_t> HttpResponse::buildResponseBuffer()
// {
//     std::vector<uint8_t> response;

//     // 1 status line example : HTTP/1.1 200 OK
//     std::ostringstream oss;
//     oss << version << " " << statusCode << " " << reasonPhrase << "\r\n";
//     response.insert(response.end(), oss.str().begin(), oss.str().end());

//     //2 APPEND headers !!
//     for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
//     {// Clear the buffer
//         oss.str(""); 
//         oss << it->first << ": " << it->second << "\r\n";
//         response.insert(response.end(), oss.str().begin(), oss.str().end());
//     }

//     response.push_back('\r');
//     response.push_back('\n');

//     // add body 
//     response.insert(response.end(), body.begin(), body.end());
//     return response;
// }
















