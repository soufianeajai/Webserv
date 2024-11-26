#include "HttpResponse.hpp"
#include "../Connection/Connection.hpp"

std::string HttpResponse::getMimeType(const std::string& filePath) const
{
    size_t pos = filePath.find_last_of('.');
    if (pos == std::string::npos)
        return "application/octet-stream";
    std::string extension = filePath.substr(pos + 1);
    std::map<std::string, std::string>::const_iterator it = mimeTypes.find(extension);
    if (it != mimeTypes.end())
        return it->second;
    return "application/octet-stream";
}


void HttpResponse::checkIfCGI( std::string& path,std::map<std::string, std::string> ExtensionsConfig)
{
    for (std::map<std::string, std::string>::const_iterator it = ExtensionsConfig.begin(); it != ExtensionsConfig.end(); ++it)
    {
        const std::string& ext = it->first;
        size_t pos = path.rfind(ext);
        if (pos != std::string::npos &&
            (pos + ext.length() == path.length() || path[pos + ext.length()] == '/') )
        {
            cgi = true;
            PATH_INFO = path.substr(pos + ext.length());
            path = path.substr(0, pos + ext.length());
            PathCmd = it->second;
            if (access(PathCmd.c_str(), F_OK) != 0 || access(PathCmd.c_str(), X_OK) != 0)
                PathCmd = "";
            break; 
        }
    }
}

bool parseCGIOutput(const std::vector<uint8_t>& cgiOutput, std::vector<uint8_t>& headers, std::vector<uint8_t>& body)
{
    const uint8_t delimiter[] = {'\r', '\n', '\r', '\n'};
    const size_t delimiterLength = 4;

    for (size_t i = 0; i + delimiterLength <= cgiOutput.size(); ++i)
    {
        if (cgiOutput[i] == delimiter[0] &&
            cgiOutput[i + 1] == delimiter[1] &&
            cgiOutput[i + 2] == delimiter[2] &&
            cgiOutput[i + 3] == delimiter[3])
        {
            size_t headerEnd = i + delimiterLength;
            headers.assign(cgiOutput.begin(), cgiOutput.begin() + headerEnd);
            if (headerEnd < cgiOutput.size())
                body.assign(cgiOutput.begin() + headerEnd, cgiOutput.end());
            else
                body.clear();
            return true;
        }
    }
    body.assign(cgiOutput.begin(), cgiOutput.end());
    return false;
}

std::string trim(const std::string& str)
{
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
        ++start;

    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        --end;
    return str.substr(start, end - start);
}

void HttpResponse::ExtractHeaders()
{
    std::vector<uint8_t> heads;
    if (parseCGIOutput(cgiOutput,heads,body))
    {
        std::string headerStr(heads.begin(), heads.end());
        size_t startPos = 0;
        size_t endPos = 0;
        while ((endPos = headerStr.find("\r\n", startPos)) != std::string::npos)
        {
            std::string line = headerStr.substr(startPos, endPos - startPos); 
            startPos = endPos + 2;
            if (line.empty())
                continue; 
            size_t delimiterPos = line.find(":");  
            if (delimiterPos != std::string::npos)
            {
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));
                headers[key] = value;
            }                 
        }        
    }
    headers["Content-Length"] =  intToString(body.size());
    totaSize = body.size();
}

int HttpResponse::parentProcess()
{
    int status;
    char buffer[1024];
    ssize_t bytesRead;
    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
    pid_t res = waitpid(pid, &status, WNOHANG);

    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        cgiOutput.insert(cgiOutput.end(), buffer, buffer + bytesRead);
    if (res > 0) 
    {
        if (WIFEXITED(status))
        {   
            if (WEXITSTATUS(status) != 0)
                return(std::cerr << "[Error] ... CGI exited with error code: " << WEXITSTATUS(status) << std::endl,close(pipefd[0]),2);
        }
        else
            return(std::cerr << "[Error] ... CGI did not terminate normally.\n",close(pipefd[0]),2);
        close(pipefd[0]);
        ExtractHeaders();
        ChildFInish =  true;
        return 0;
    }
    if (static_cast<time_t>(time(NULL)) - currenttime > TIMEOUT)
        return 1;
    return -1;
}

int HttpResponse::executeCGI()
{
    if (pipe(pipefd) == -1)
        return(std::cout << "[Error] ... : "<<strerror(errno)<<std::endl,1);
    pid = fork();
    if (pid == -1)
        return(std::cout << "[Error] ...: "<<strerror(errno)<<std::endl,1);
    if (pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); 
        char* argv[] = {const_cast<char*>(PathCmd.c_str()), const_cast<char*>(Page.c_str()), NULL};
        if (execve(PathCmd.c_str(), argv, &envVars[0]) == -1)
        {
            std::cout << "[Error] ... : "<<strerror(errno)<<std::endl;
            close(pipefd[1]);
            exit(1);
        }
    }
    else
        close(pipefd[1]); 
    return 0;
}



void HttpResponse::createEnvChar(HttpRequest& request, std::string& uri,const std::string& host,const std::string& port)
{
    envVars.push_back(strdup("GATEWAY_INTERFACE=CGI/1.1"));
    envVars.push_back(strdup(("REQUEST_METHOD=" + request.getMethod()).c_str()));
    envVars.push_back(strdup(("SCRIPT_NAME=" + uri).c_str()));
    envVars.push_back(strdup(("PATH_INFO=" + PATH_INFO).c_str()));
    envVars.push_back(strdup(("SCRIPT_FILENAME=" + Page).c_str()));
    envVars.push_back(strdup(("HTTP_COOKIE=" + Cookies).c_str()));
    envVars.push_back(strdup(("PHP_SELF=" + uri).c_str()));
    envVars.push_back(strdup(("PATH_TRANSLATED=" + Page).c_str()));
    envVars.push_back(strdup(("SERVER_NAME=" + host).c_str()));
    envVars.push_back(strdup(("SERVER_PORT=" + port).c_str()));
    envVars.push_back(strdup(("SERVER_PROTOCOL=" + version).c_str()));
    envVars.push_back(strdup("SERVER_SOFTWARE=MyWebServer/1.0"));
    envVars.push_back(strdup("REDIRECT_STATUS=200"));
    envVars.push_back(strdup(("QUERY_STRING=" + request.getQuery()).c_str()));
    envVars.push_back(strdup("DB_PATH=Posted_Data"));
    for (int i = 0; environ[i] != NULL; ++i)
       envVars.push_back(strdup(environ[i])); 
    envVars.push_back(NULL);   
}


void HttpResponse::sendCgi(int clientSocketId, Status& status)
{
    int SentedBytes;
    size_t chunkSize = (totaSize - offset < Connection::CHUNK_SIZE) 
                   ? (totaSize - offset) 
                   : Connection::CHUNK_SIZE;
    SentedBytes = send(clientSocketId, &body[offset], chunkSize, MSG_NOSIGNAL);
    if (SentedBytes < 0)
    { 
         std::cerr << "Send failed to client " << clientSocketId 
                  << ": " << strerror(errno) << std::endl;
        status = DONE;  // handle error as needed
        return;
    }
    if (SentedBytes == 0) 
    {
        // If `send` returns 0, it means the connection might be closed
        std::cerr << "Connection closed by client " << clientSocketId << std::endl;
        status = DONE;
        return;
    }
    
    offset += SentedBytes;
    if (offset >= static_cast<size_t>(totaSize))
    {
        status = DONE;
        return;
    }
}