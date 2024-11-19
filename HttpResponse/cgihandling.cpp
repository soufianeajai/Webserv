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
    return "application/octet-stream";  // Default type if extension not found
}

void HttpResponse::GetFullPathCmd(const std::string& ext)
{
    if (ext == ".php")
        PathCmd = "/usr/bin/php-cgi8.1";
    else
        PathCmd = "/usr/bin/python3.10";
    if (access(PathCmd.c_str(), F_OK) != 0 || access(PathCmd.c_str(), X_OK) != 0)
        PathCmd = "";
    else
        std::cout << "found path cmd !\n";
}

void HttpResponse::checkIfCGI(HttpRequest& request, std::string& path, std::set<std::string> ExtensionsConfig, std::string& uri,const std::string& host,const std::string& port)
{
    for (std::set<std::string>::const_iterator it = ExtensionsConfig.begin(); it != ExtensionsConfig.end(); ++it)
    {
        const std::string& ext = *it;
        size_t pos = path.rfind(ext);
        //extension is not at the end or followed by a '/' its invalid like : /cgi/somefile.php.invalid
        // need to update to 501 for cgi exist but is not in my configfile 
        if (pos != std::string::npos &&
            (pos + ext.length() == path.length() || path[pos + ext.length()] == '/') && (ext == ".php" || ext == ".py"))
        {
            cgi = true;
            PATH_INFO = path.substr(pos + ext.length());
            path = path.substr(0, pos + ext.length());
            GetFullPathCmd(ext);
            break; 
        }
    }
    if (cgi)
        createEnvChar(request, uri, host, port);
    for (size_t i = 0; i < envVars.size() && envVars[i] != NULL; ++i)
        std::cout << envVars[i] << std::endl;
}

/* Find the position of the '?' and pos of / to get script name
        /cgi/script.php/test/more/path?param1=value1&param2=value2
                                     pos + 1 = p
            queryString =   param1=value1&param2=value2
            scriptName =    /cgi/script.php
            path_info = /test/more/path                
    */

int HttpResponse::executeCGI(time_t currenttime)
{
    int status;
    std::string body;
    std::string headers;
    //std::string cgiOutput;
    //int original_output = dup(STDOUT_FILENO);
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return 500;
    }
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return 500;
    }
    if (pid == 0)
    {
        close(pipefd[0]);
        std::cout <<"child : cmd : "<<PathCmd<<" Page :"<<Page<<"\n";
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); 
        char* argv[] = {const_cast<char*>(PathCmd.c_str()), const_cast<char*>(Page.c_str()), NULL};
        execve(PathCmd.c_str(), argv, &envVars[0]);
        perror("execve failed");
        exit(1);
    }
    else
    {
        close(pipefd[1]);
        while(true)
        {
            pid_t res = waitpid(pid, &status, WNOHANG);
            std::cout <<"res: "<<res<<" , elasped: "<<(static_cast<time_t>(time(NULL)) - currenttime)<<"\n";
            if (res == -1)
            {
                perror("waitpid failed");
                close(pipefd[0]);
                return 500;
            }
            if (res > 0) 
                break;
           if (static_cast<time_t>(time(NULL)) - currenttime > TIMEOUT)
            {
                kill(pid, SIGKILL);
                waitpid(pid, &status, 0);
                close(pipefd[0]);
                return 500;
            }
        }
        if (WIFEXITED(status))
        {   
            if (WEXITSTATUS(status) != 0)
            {
                std::cerr << "CGI script exited with error code: " << WEXITSTATUS(status) << std::endl;
                close(pipefd[0]);
                return 500;
            }
        }
        else
        {
            std::cerr << "CGI script did not terminate normally." << std::endl;
            close(pipefd[0]);
            return 500;
        }
        std::cout << "timecurrent: "<<currenttime<<" time now: "<<static_cast<time_t>(time(NULL))<< " def: "<<static_cast<time_t>(time(NULL)) - currenttime<<"\n";
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
            cgiOutput.append(buffer, buffer + bytesRead);
        if (bytesRead == -1)
        {
            perror("read failed");
            close(pipefd[0]);
            return 500;
        }
    close(pipefd[0]);
    totaSize = cgiOutput.size();
    return 200;    // size_t headerEnd = cgiOutput.find("\r\n\r\n");
    // if (headerEnd != std::string::npos)
    // {
    //     std::cout << "waaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n";
    //     headers = cgiOutput.substr(0, headerEnd);  // Extract headers
    //     body = cgiOutput.substr(headerEnd + 4);  // Extract body
    // }
    }
}



void HttpResponse::createEnvChar(HttpRequest& request, std::string& uri,const std::string& host,const std::string& port)
{
    
    envVars.push_back(strdup("GATEWAY_INTERFACE=CGI/1.1"));
    envVars.push_back(strdup(("REQUEST_METHOD=" + request.getMethod()).c_str()));

    envVars.push_back(strdup(("SCRIPT_NAME=" + uri).c_str()));
    envVars.push_back(strdup(("PATH_INFO=" + PATH_INFO).c_str()));
    envVars.push_back(strdup(("PATH_TRANSLATED=" + Page).c_str()));

    envVars.push_back(strdup(("SERVER_NAME=" + host).c_str()));
    envVars.push_back(strdup(("SERVER_PORT=" + port).c_str()));
    envVars.push_back(strdup(("SERVER_PROTOCOL=" + version).c_str()));
    envVars.push_back(strdup("SERVER_SOFTWARE=MyWebServer/1.0"));
    envVars.push_back(strdup("REDIRECT_STATUS=200"));
    // get
    envVars.push_back(strdup(("QUERY_STRING=" + request.getQuery()).c_str()));
    // post
    if (request.getMethod() == "POST")
    {
        envVars.push_back(strdup(("CONTENT_TYPE=" + request.getHeader("CONTENT_TYPE")).c_str()));
        envVars.push_back(strdup(("CONTENT_LENGTH=" + request.getHeader("CONTENT_LENGTH")).c_str()));
    }
    envVars.push_back(NULL);
}


void HttpResponse::sendCgi(int clientSocketId, Status& status)
{
    //we have totalSize, and offset = 0
    int SentedBytes;
    if (offset >= static_cast<size_t>(totaSize))
    {
        status = DONE;
        return;
    }
    size_t chunkSize = (totaSize - offset < Connection::CHUNK_SIZE) 
                   ? (totaSize - offset) 
                   : Connection::CHUNK_SIZE;
    SentedBytes = send(clientSocketId, &cgiOutput[offset], chunkSize, MSG_NOSIGNAL);
    if (SentedBytes < 0)
    { 
        std::cerr << "3 Send failed to client "<<clientSocketId << std::endl;
        status = DONE;  // handle error as needed
        return;
    }
    offset += SentedBytes;
}