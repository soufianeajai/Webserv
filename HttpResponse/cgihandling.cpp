#include "HttpResponse.hpp"

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

void HttpResponse::checkIfCGI(const std::string& path, std::set<std::string> ExtensionsConfig)
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
            //std::cout << "cgi-> path: "<<path<<" path[pos]: "<<path[pos + 1]<<"\n";
            break; 
        }
    }
}

/* Find the position of the '?' and pos of / to get script name
        /cgi/script.php/test/more/path?param1=value1&param2=value2
                                     pos + 1 = p
            queryString =   param1=value1&param2=value2
            scriptName =    /cgi/script.php
            path_info = /test/more/path                
    */

// void HttpResponse::executeCGI(const std::string& scriptPath,std::vector<char*> &envp)
// {
    
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
//         execve(scriptPath.c_str(), argv, &envp[0]);
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



std::vector<char*> HttpResponse::createEnvChar(HttpRequest& request)
{
    std::vector<char*> envVars;

    std::string scriptName;

    std::string contentType;
    //scriptName = ;

    
    // query done!
    //  CONTENT_TYPE is have POST data sinon empty
    // CONTENT_LENGTH (body request)
    // PATH_INFO : parse url get string after extention and PATH_TRANSLATED 
    //

    //

    // REQUEST_METHOD from request
    //SCRIPT_NAME = /cgi/script.php (url) The SCRIPT_NAME variable MUST be set to a URI path (The virtual path to the CGI script.)

    //SERVER_PROTOCOL, SERVER_PROTOCOL = HTTP-Version (version exist in base class)
    
    
    envVars.push_back(const_cast<char*>(("REQUEST_METHOD=" + request.getMethod()).c_str()));
    envVars.push_back(const_cast<char*>(("QUERY_STRING=" + request.getQuery()).c_str()));
    
    envVars.push_back(const_cast<char*>(("SCRIPT_NAME=" + scriptName).c_str()));


    envVars.push_back(const_cast<char*>(("CONTENT_TYPE=" + request.getHeader("CONTENT_TYPE")).c_str()));
    /*
    get it from server !!
    SERVER_NAME =  hostname | ipv4-address | ( "[" ipv6-address "]" ) , 
    SERVER_PORT=80 , get from server !
    */
    envVars.push_back(const_cast<char*>(("SERVER_NAME=" + request.getUri()).c_str()));
    envVars.push_back(const_cast<char*>(("SERVER_PORT=" + request.getUri()).c_str()));
    //--------------------------------------------------------------------------------
    envVars.push_back(const_cast<char*>(("SCRIPT_NAME=" + request.getUri()).c_str()));
    envVars.push_back(const_cast<char*>(("SERVER_PROTOCOL=" + version).c_str()));
    envVars.push_back(const_cast<char*>("GATEWAY_INTERFACE=CGI/1.1"));
    envVars.push_back(NULL);
    return envVars;
}


//char* const args[] = { const_cast<char*>(scriptPath.c_str()), NULL };
// if (execve(scriptPath.c_str(), args, env.data()) == -1) {
//         perror("execve failed");
//         return 1;  // Return error if execve fails
//     }