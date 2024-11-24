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
// size_t HttpResponse::getOffset()
// {
//     return offset;
// }
bool HttpResponse::getCgi() const
{
    return cgi;
}
pid_t HttpResponse::getPid() const
{
    return pid;
}
// int HttpResponse::getpipe() const
// {
//     if (cgi)
//         return pipefd[0];
//     return -1;
// }

std::string getPWDVariable()
{
    for (char** current = environ; *current != NULL; ++current)
    {
        if (std::strncmp(*current, "PWD=", 4) == 0)
            return std::string(*current + 4);
    }
    return ""; // Return an empty string if PATH is not found
}

HttpResponse::HttpResponse():totaSize(0),offset(0),headerSended(false),cgi(false),PathCmd(""),PWD(getPWDVariable()),pid(-1),currenttime(0)
{
    ChildFInish = false;
    cgiOutput.clear();
    envVars.clear();
    body.clear();
    headers.clear();
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
    time(&rawTime);
    timeInfo = gmtime(&rawTime);
    strftime(buffer, 80, "%a, %d %b %Y %H:%M:%S GMT", timeInfo);
    return std::string(buffer);
}

void HttpResponse::handleRedirection(const Route &route)
{
    //Location: https://example.com/new-path ~~!!!
    if (route.getIsRedirection() && !route.getNewPathRedirection().empty())
    {
        UpdateStatueCode(route.getstatusCodeRedirection());
        Page = PWD + route.getNewPathRedirection();
    }
    else
        UpdateStatueCode(404);
}

void HttpResponse::UpdateStatueCode(int code)
{
    statusCode = code;
    cgi = false;
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
        case 501: reasonPhrase = "Not Implemented"; break; // for Unsupported CGI Extension
        case 504: reasonPhrase = "Gateway Timeout";break;
        case 505: reasonPhrase = "HTTP Version Not Supported"; break;
        case 201: reasonPhrase = "Created"; break;
        case 204: reasonPhrase = "No Content"; break;
        default:  reasonPhrase = "OK"; break;
    }

    std::map<int, std::string>::iterator it = defaultErrors.find(statusCode);
    if (it != defaultErrors.end())
        Page = it->second;
    else
        Page = DEFAULTERROR;
    std::ifstream file(Page.c_str());
    if (!file.is_open())
    {
        totaSize = -1; // this is last part we can do after check for error response , set it to -1 , then we check it in send response if equal to -1 so close connection no body for client !!
        return ;
    }
    file.seekg(0, std::ios::end);
    totaSize = file.tellg();
    file.seekg(0, std::ios::beg);
    file.close();
    headers["Content-Length"] =  intToString(totaSize);
    headers["Content-Type"] = "text/html";
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

void HttpResponse::handleRequest(std::string& host, uint16_t port,HttpRequest & request)
{
    UpdateStatueCode(request.GetStatusCode());
    std::string uri = request.getUri();
    Route& route = request.getCurrentRoute();
    route.setRoot(PWD + route.getRoot());
    if(request.getUri().empty())
        uri = request.getUri() + "/";
    if (statusCode ==  200 || statusCode ==  201)
    {
        if (route.getPath() == uri)
        {
            if(!route.getDefaultFile().empty())
            {
                Page =  route.getRoot() + "/" + route.getDefaultFile();
                uri += (uri[uri.size() - 1] != '/') ? "/" +  route.getDefaultFile() : route.getDefaultFile(); // need delete this
            }
            else if(route.getAutoindex())
                HandleIndexing(route.getRoot(),request.getUri());
            else
                UpdateStatueCode(404);
        }
        else
        {
            if (route.getPath() != "/" && uri.find(route.getPath()) == 0)
            {
                uri.erase(0, route.getPath().size());
                Page = route.getRoot() + uri;
                uri = request.getUri();
            }
            else
                UpdateStatueCode(404);
        }
        checkIfCGI(Page, route.getCgiExtensions());
    }
    else if (statusCode ==  204)
        std::cout << "[DELETE data]\n";
    if(route.getIsRedirection())
        handleRedirection(route);
    CheckExistingInServer();
    if (cgi)
        createEnvChar(request, uri, host, intToString(port));
}


void HttpResponse::ResponseGenerating(HttpRequest & request, std::map<int, std::string> &errorPages
                    , Status& status,std::string& host, uint16_t port, time_t currenttime)
{
    this->currenttime = currenttime;
    defaultErrors = errorPages;
    version = request.getVersion();
    handleRequest(host,port, request);
    std::cout << "page : "<<Page<<" , cgi detected : "<<cgi<<"\n";
    if (cgi)
    {
        int res = executeCGI();
        if (res == 1)
            UpdateStatueCode(500);
    }
    else
    {
        headers["Content-Type"] = getMimeType(Page);
        headers["Content-Length"] = intToString(totaSize);
    }
    headers["Date"] =  getCurrentTimeFormatted();
    headers["Server"] =  "WebServ 1337";  
    headers["Connection"] = "close";
    status = SENDING_RESPONSE;
}


void HttpResponse::CheckExistingInServer()
{
    std::ifstream file(Page.c_str());
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        totaSize = file.tellg();
        file.seekg(0, std::ios::beg);
        //std::cout <<"\n!!!!!!!!!!!!!!!! file open is in my server : "<<Page.c_str()<<"\n";
        file.close();
    }
    else
    {
        UpdateStatueCode(404);
        //std::cout <<"\n!!!!!!!!!!!!!!!! file not exist in my server : "<<Page.c_str()<<"\n";  
    }
}















