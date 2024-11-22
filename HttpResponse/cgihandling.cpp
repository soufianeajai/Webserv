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


void HttpResponse::checkIfCGI(HttpRequest& request, std::string& path,std::map<std::string, std::string> ExtensionsConfig, std::string& uri,const std::string& host,const std::string& port)
{
    for (std::map<std::string, std::string>::const_iterator it = ExtensionsConfig.begin(); it != ExtensionsConfig.end(); ++it)
    {
        const std::string& ext = it->first;
        size_t pos = path.rfind(ext);
        //extension is not at the end or followed by a '/' its invalid like : /cgi/somefile.php.invalid
        // need to update to 501 for cgi exist but is not in my configfile 
        //&& (ext == ".php" || ext == ".py")
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
    if (cgi)
        createEnvChar(request, uri, host, port);
    // for (size_t i = 0; i < envVars.size() && envVars[i] != NULL; ++i)
    //     std::cout << envVars[i] << std::endl;
}

/* Find the position of the '?' and pos of / to get script name
        /cgi/script.php/test/more/path?param1=value1&param2=value2
                                     pos + 1 = p
            queryString =   param1=value1&param2=value2
            scriptName =    /cgi/script.php
            path_info = /test/more/path                
    */

int HttpResponse::parentProcess(time_t currenttime)
{
    int status;
    char buffer[1024];
    ssize_t bytesRead;
    pid_t res = waitpid(pid, &status, WNOHANG);
    if (res == -1)
        return(std::cout << "Error pipe no: "<<strerror(errno)<<std::endl,close(pipefd[0]),1);
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer))) > 0)
        cgiOutput.append(buffer, buffer + bytesRead);

    if (static_cast<time_t>(time(NULL)) - currenttime > TIMEOUT)
        return (kill(pid, SIGKILL),close(pipefd[0]), 1); // need update it to timeout page !!
    if (res > 0) 
    {
        std::cout << "finish child\n";
        if (WIFEXITED(status))
        {   
            if (WEXITSTATUS(status) != 0)
                return(std::cerr << "CGI exited with error code: " << WEXITSTATUS(status) << std::endl,close(pipefd[0]),500);
        }
        else
            return(std::cerr << "CGI did not terminate normally.\n",close(pipefd[0]),500);
        close(pipefd[0]);
        totaSize = cgiOutput.size();
        return 0;
    }
    return -1;
}

int HttpResponse::executeCGI()
{
    fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
    if (pipe(pipefd) == -1)
        return(std::cout << "Error: "<<strerror(errno)<<std::endl,1);
    pid = fork();
    if (pid == -1)
        return(std::cout << "Error: "<<strerror(errno)<<std::endl,1);
    if (pid == 0)
    {
        std::cout << "path: "<<PathCmd<<"\n";
        std::cout << "path: "<<Page<<"\n";
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]); 
        char* argv[] = {const_cast<char*>(PathCmd.c_str()), const_cast<char*>(Page.c_str()), NULL};
        execve(PathCmd.c_str(), argv, &envVars[0]);
        std::cout << "Error: "<<strerror(errno)<<std::endl;
        exit(1);
    }
    close(pipefd[1]);
    return 0;
}



void HttpResponse::createEnvChar(HttpRequest& request, std::string& uri,const std::string& host,const std::string& port)
{
    
    //REMOTE_PORT,REMOTE_ADDR
    // envVars.push_back(strdup("REMOTE_PORT=12345"));
    // envVars.push_back(strdup("REMOTE_ADDR=127.0.0.1"));

    envVars.push_back(strdup("GATEWAY_INTERFACE=CGI/1.1"));
    envVars.push_back(strdup(("REQUEST_METHOD=" + request.getMethod()).c_str()));
    envVars.push_back(strdup(("SCRIPT_NAME=" + uri).c_str()));
    envVars.push_back(strdup(("PATH_INFO=" + PATH_INFO).c_str()));

    //SCRIPT_FILENAME
    envVars.push_back(strdup(("SCRIPT_FILENAME=" + Page).c_str()));
    //DOCUMENT_ROOT
    //envVars.push_back(strdup(("DOCUMENT_ROOT=" + environ["PATH"])));
    //PHP_SELF
    envVars.push_back(strdup(("PHP_SELF=" + uri).c_str()));
    
    envVars.push_back(strdup(("PATH_TRANSLATED=" + Page).c_str()));
    envVars.push_back(strdup(("SERVER_NAME=" + host).c_str()));
    envVars.push_back(strdup(("SERVER_PORT=" + port).c_str()));
    envVars.push_back(strdup(("SERVER_PROTOCOL=" + version).c_str()));
    envVars.push_back(strdup("SERVER_SOFTWARE=MyWebServer/1.0"));
    envVars.push_back(strdup("REDIRECT_STATUS=200"));
    // get
    envVars.push_back(strdup(("QUERY_STRING=" + request.getQuery()).c_str()));
    envVars.push_back(strdup(("CONTENT_TYPE=" + request.getHeader("CONTENT_TYPE")).c_str()));
    envVars.push_back(strdup(("CONTENT_LENGTH=" + request.getHeader("CONTENT_LENGTH")).c_str()));
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
    SentedBytes = send(clientSocketId, &cgiOutput[offset], chunkSize, MSG_NOSIGNAL);
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