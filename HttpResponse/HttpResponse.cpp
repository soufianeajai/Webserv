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

bool checkIfCGI(const std::string& url)
{
    size_t extPos = url.rfind('.');
    if (extPos != std::string::npos)
    {
        std::string extension = url.substr(extPos);
        return cgiExtensions.find(extension) != cgiExtensions.end();
    }
    return false;
}

std::vector<char*> HttpResponse::createEnvChar(const std::string& Url, ...) const 
{
    std::vector<char*> envVars;
    bool cookies = false; // TODO detect if have cookies session manangement
    
    std::string authType = cookies ? "SessionCookie" : "";
    envVars.push_back(strdup(("AUTH_TYPE=" + authType).c_str()));

    std::string method = "GET"; // FROM PARAM FUNCTION
    envVars.push_back(strdup(("CONTENT_LENGTH=" + intToString(request.GetBody.size())).c_str())); // Length of POST data
    
    

    return envVars;
}
void HttpResponse::executeCGI(const std::string& scriptPath,std::vector<char*> &envp)
{
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return;
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }
    if (pid == 0)
    {
        // In child process
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]); 
        char* argv[] = {const_cast<char*>(scriptPath.c_str()), NULL};
        execve(scriptPath.c_str(), argv, &envp[0]); // envp.data() but is in c++11
    }
    else
    {
        // In parent process
        close(pipefd[1]);
        std::string cgiOutput;
        char buffer[1024];
        ssize_t bytesRead;
         while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
            cgiOutput.append(buffer, buffer + bytesRead);
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        body.assign(cgiOutput.begin(), cgiOutput.end());
    }
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



HttpResponse::HttpResponse(int index_connection, const Server &server):Pages(server.getErrorPages())
{ 
    // id of connection to get request also get same data  from config file 
    statusCode = server.connections[index_connection].request.GetStatusCode();
    switch (statusCode)
    {  
        case 400: reasonPhrase = "Bad Request"; break;
        case 403: reasonPhrase = "Forbidden"; break;
        case 404: reasonPhrase = "Not Found"; break;
        case 405: reasonPhrase = "Method Not Allowed"; break;
        case 500: reasonPhrase = "Internal Server Error"; break;
        case 505: reasonPhrase = "HTTP Version Not Supported"; break;
        default: Pages[statusCode] = server.connections[index_connection].request.GetUrl(); break;
    }

    // store mime types 
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

// set best type of mime for content-type !!
std::string HttpResponse::getMimeType(const std::string& filePath) const
{
    size_t pos = filePath.find_last_of('.');
    if (pos == std::string::npos)
        return "application/octet-stream";
    std::string extension = filePath.substr(pos + 1);
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
    if (it != mimeTypes.end())
        return it->second;
    return "application/octet-stream";  // Default type if extension not found
}

void HttpResponse::generateResponse(std::string Url, bool flagConnection,... )
{
    // detect CGI    
    if (checkIfCGI(Url))
        executeCGI(Url, createEnvMap(Url));  // createEnvMap builds necessary env variables
    else
        LoadPage();
    addHeader("Content-Type", getMimeType(Url));
    
    //Transfer-Encoding: chunked or content-length ?
    addHeader("Content-Length", intToString(body.size()));
    
    
    addHeader("Date", getCurrentTimeFormatted());
    addHeader("Server", "WebServ 1337");  
    addHeader("Connection",flagConnection ? "keep-alive" : "close");

    //Location: https://example.com/new-path
    // from config file  : redirect: old-path 3xx new-path
    //The browser automatically makes a new HTTP request to the URL specified in the Location header
    if (statusCode > 299 && statusCode < 400)
        addHeader("Location", "localhost?" + "new-path");

    // cookies header ?
    // addHeader("Set-Cookie",createSetCookieHeader());
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

