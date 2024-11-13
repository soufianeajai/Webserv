#include "HttpResponse.hpp"
std::string intToString(size_t number);
void SendData(std::vector<uint8_t> data);
std::string getCurrentTimeFormatted();
// #include <sys/stat.h>
// #include "../HttpRequest/processRequest.cpp"
HttpResponse::HttpResponse():sendbytes(0){}

size_t HttpResponse::getSendbytes()
{
    return sendbytes;
}

void HttpResponse::addToSendbytes(size_t t)
{
    sendbytes +=t;
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
// this function is in  processRequest
// bool isDirectory(const std::string& path)
// {
//     struct stat pathStat;
//     if (stat(path.c_str(), &pathStat) != 0)
//     {
//         perror("stat");  // Print an error if stat fails
//         return false;    // Treat as non-directory if there's an error
//     }
//     return S_ISDIR(pathStat.st_mode);  // Check if it's a directory
// }

// std::string createSetCookieHeader(const std::string& sessionId)
// {
//         return "session_id=" + sessionId + "; Path=/; HttpOnly";
// }



//1xx (Informational): The request was received, continuing process

//2xx (Successful): The request was successfully received,
//understood, and accepted

//3xx (Redirection): Further action needs to be taken in order to
//complete the request

//4xx (Client Error): The request contains bad syntax or cannot be
//fulfilled

//5xx (Server Error): The server failed to fulfill an apparently
//valid request



void HttpResponse::LoadPage()
{
    try{
    std::ifstream file(Page.c_str());
    if (file.is_open()) // always is true 
    {
         //std::cout << "\n\nbody  from : " << Page <<"\n\n";
        body.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
    else
    {
        //std::cout << "\n\nno body\n\n";
        body.clear();
    }
    }
    catch (std::exception &e)
    {
        std::cout << e.what()<<std::endl;
    }
}


void HttpResponse::handleError(std::map<int, std::string>& errorPages)
{
    std::map<int, std::string>::iterator it = errorPages.find(statusCode);
        if (it != errorPages.end())
            Page = it->second;
        else
            Page = DEFAULTERROR;
        std::cout << "page error" << Page << std::endl;
}

void HttpResponse::handleRedirection(const Route &route)
{
    
    if (route.getIsRedirection() && !route.getNewPathRedirection().empty())
    {
        UpdateStatueCode(route.getstatusCodeRedirection());
        Page = route.getRoot() + route.getNewPathRedirection(); // path valid trust from process request
    }
    else
    {
        UpdateStatueCode(route.getstatusCodeRedirection());
        Page = DEFAULTERROR;  // Fallback to default error page if redirection path not set
    }
}

std::vector<uint8_t> HttpResponse::buildResponseBuffer()
{
    std::vector<uint8_t> response;
    try{
    // 1 status line example : HTTP/1.1 200 OK
    std::ostringstream oss;

    // 1. Append the status line (e.g., HTTP/1.1 200 OK)
    std::cout <<"\n\nbuildResponseBuffer : "<< version << " " << statusCode << " " << reasonPhrase <<".\n\n";
    oss << version << " " << statusCode << " " << reasonPhrase << "\r\n";

    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }

    oss << "\r\n";
    std::string responseStr = oss.str();
    response.insert(response.end(), responseStr.begin(), responseStr.end());

    // add body 
    response.insert(response.end(), body.begin(), body.end());
    
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in buildResponseBuffer: " << e.what() << std::endl;
        // Handle any other standard exceptions that may occur
    }
    return response;
}


void HttpResponse::UpdateStatueCode(int code)
{
    statusCode = code;
    switch (statusCode)
    {  
        case 100: reasonPhrase = "Continue"; break; // The server expects the client to continue sending the request, and the body is empty in this response.
        case 301: reasonPhrase = "Moved Permanently"; break;
        case 302: reasonPhrase = "Found"; break;  // Also called "Temporary Redirect" in some contexts
        case 303: reasonPhrase = "See Other"; break;
        case 307: reasonPhrase = "Temporary Redirect"; break;
        case 308: reasonPhrase = "Permanent Redirect"; break;
        case 400: reasonPhrase = "Bad Request"; break;
        case 403: reasonPhrase = "Forbidden"; break;
        case 404: reasonPhrase = "Not Found"; break;
        case 405: reasonPhrase = "Method Not Allowed"; break;
        case 500: reasonPhrase = "Internal Server Error"; break;
        /*
            for 500 :
            Scenario:
            The server has the necessary CGI functionality installed 
            (e.g., PHP-FPM or another CGI handler for PHP), 
            but the location configuration in the server (e.g., NGINX)
             does not specify how to handle .php files.

            501
            example : 
            It would indicate that the CGI functionality is not implemented 
            for that file extension or type.
        */
        case 501: reasonPhrase = "501 Not Implemented"; break; // for Unsupported CGI Extension
        case 505: reasonPhrase = "HTTP Version Not Supported"; break;
        case 201: reasonPhrase = "Created"; break;
        case 204: reasonPhrase = "No Content"; break;
        default:  reasonPhrase = "OK"; break;
    }
}
void HttpResponse::GeneratePageIndexing(std::string& fullpath,std::string& uri, std::vector<std::string>& files)
{
    Page = DEFAULTINDEX;
    std::ofstream file(DEFAULTINDEX, std::ios::out | std::ios::trunc);
    if (!file)
    {
        std::cerr << "Error: Could not open file for writing: " << uri << std::endl;
        UpdateStatueCode(404);
        return;
    }
    std::ostringstream html;
    html << "<html><head><title>Index of " << fullpath << "</title></head><body>";
    html << "<h1>Index of " << fullpath << "</h1><ul>";
    for (size_t i = 0; i < files.size(); ++i)
        html << "<li><a href=\"" << files[i] << "\">" << files[i] << "</a></li>";
    html << "</ul></body></html>";

    file << html.str();
    file.close();
}

void HttpResponse::HandleIndexing(std::string fullpath, std::string& uri)
{
    struct stat pathStat;
    if (stat(fullpath.c_str(), &pathStat) != 0 || !S_ISDIR(pathStat.st_mode))
    {
        std::cerr << "Error: Path does not exist or is not a directory." << std::endl;
        UpdateStatueCode(404);
        return;
    }
    // Open the directory
    DIR* dir = opendir(fullpath.c_str());
    if (!dir) {
        std::cerr << "Error: Unable to open directory." << std::endl;
        UpdateStatueCode(404);
        return;
    }

    std::vector<std::string> files;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if (name != "." && name != "..")
            files.push_back(name);
    }
    closedir(dir);
    GeneratePageIndexing(fullpath,uri, files);
}

std::vector<uint8_t> HttpResponse::ResponseGenerating(HttpRequest & request, std::map<int, std::string> &errorPages)
{

    Route& route = request.getCurrentRoute();
    this->query = request.getQuery();
    version = request.getVersion();
    UpdateStatueCode(request.GetStatusCode());
    route.setRoot("." + route.getRoot());
    ValidcgiExtensions.insert(".php");
    ValidcgiExtensions.insert(".py");
    ValidcgiExtensions.insert(".sh");
    // then compare if we have in config file so we can execute it sinon (500 or 501 status code)
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
    
    if(route.getIsRedirection())
        handleRedirection(route);
    Page = "www/html/index.html";
    //std::cout << "static file !! : "<<route.getPath()<<"__"<<Page<<"__"<<route.isDirGetter()<< std::endl;
    if(request.getUri().empty())
        request.getUri() = request.getUri() + "/";
    std::cout << "get path : "<<route.getPath()<<"\nUri: "<<request.getUri()<<"\nget root: "<<route.getRoot()<<"\nis dir route: "<<route.isDirGetter()<<"\ndefualt file: "<<route.getDefaultFile()<<"\nauto index: "<<route.getAutoindex()<<"\n";
    if (statusCode ==  200) // GET
    {
        if (route.getPath() == request.getUri())
        {
            std::cout << "\nexact!!\n";
            if (route.isDirGetter())
            {
                if(!route.getDefaultFile().empty())
                {
                    Page =  route.getRoot() + "/" + route.getDefaultFile();
                    std::cout << "\ndefault\n";
                }
                else if(route.getAutoindex())
                {
                    std::cout << "\nindexing: "<<route.getRoot();
                    HandleIndexing(route.getRoot(),request.getUri());
                }
            }
            else 
                Page = route.getRoot(); // alert about config file is dir or not !! (if is dir so root will be file also)
        std::cout << "\npage->>>>>> : "<<Page<<"\n";
        }
        else
            UpdateStatueCode(404);    
    }
    
    if (statusCode > 399)
        handleError(errorPages);
    LoadPage();
    headers["Content-Type"] =  getMimeType(Page);
    //Transfer-Encoding: chunked or content-length ?
    headers["Content-Length"] =  intToString(body.size());
    //std::cout <<"\n\n"<<headers["Content-Length"] <<"\n\n";
    headers["Date"] =  getCurrentTimeFormatted();
    headers["Server"] =  "WebServ 1337";  
    headers["Connection"] = "close";
    if(Page.empty())
        UpdateStatueCode(100);
    return(buildResponseBuffer());
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

    // addHeader("Content-Type", getMimeType(url); // content-type for cgi ?
    
    // //Transfer-Encoding: chunked or content-length ?
    // addHeader("Content-Length", intToString(body.size()));
    // addHeader("Date", getCurrentTimeFormatted());
    // addHeader("Server", "WebServ 1337");  
    // addHeader("Connection",connection.GetflagConnection() ? "keep-alive" : "close");

//     //Location: https://example.com/new-path
//     // from config file  : redirect: old-path 3xx new-path
//     //The browser automatically makes a new HTTP request to the URL specified in the Location header
//     if (statusCode > 299 && statusCode < 400)
//         addHeader("Location", "localhost?" + "new-path");

//     // cookies header ?
//     // addHeader("Set-Cookie",createSetCookieHeader());
// }


















