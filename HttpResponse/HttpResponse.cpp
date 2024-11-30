#include "HttpResponse.hpp"

bool HttpResponse::getCgi() const
{
    return cgi;
}
pid_t HttpResponse::getPid() const
{
    return pid;
}


HttpResponse::HttpResponse():statusCode(-1),totalSize(0),offset(0),headerSended(false),cgi(false),PathCmd(""),PWD(getPWDVariable()),pid(-1),currenttime(0)
{
    version = "HTTP/1.1" ;
    Cookies = "";
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
    if (route.getIsRedirection() && !route.getNewPathRedirection().empty())
    {
        headers["Location"] = route.getNewPathRedirection();
        UpdateStatueCode(route.getstatusCodeRedirection());
    }
}

void HttpResponse::UpdateStatueCode(int code)
{
    struct stat buffer;
    statusCode = code;
    cgi = false;
    switch (statusCode)
    {
        case 301: reasonPhrase = "Moved Permanently"; break;
        case 302: reasonPhrase = "Found"; break; 
        case 303: reasonPhrase = "See Other"; break;
        case 307: reasonPhrase = "Temporary Redirect"; break;
        case 308: reasonPhrase = "Permanent Redirect"; break;
        case 400: reasonPhrase = "Bad Request"; break;
        case 403: reasonPhrase = "Forbidden"; break;
        case 404: reasonPhrase = "Not Found"; break;
        case 413: reasonPhrase = "File Too Large"; break;
        case 405: reasonPhrase = "Method Not Allowed"; break;
        case 500: reasonPhrase = "Internal Server Error"; break;
        case 501: reasonPhrase = "Not Implemented"; break; 
        case 504: reasonPhrase = "Gateway Timeout";break;
        case 505: reasonPhrase = "HTTP Version Not Supported"; break;
        case 201: reasonPhrase = "Created"; break;
        case 204: reasonPhrase = "No Content"; break;
        case 411: reasonPhrase = "Length Required"; break;
        case 408: reasonPhrase = "Request Timeout";break;
        case 200: reasonPhrase = "OK";break;
    }
    if (statusCode >=400)
    {
        std::map<int, std::string>::iterator it = defaultErrors.find(statusCode);
        if (it != defaultErrors.end() && !stat(it->second.c_str(), &buffer))
            Page = it->second;
        else
            Page = DEFAULTERROR;
    
        std::ifstream file(Page.c_str());
        if (!file.is_open())
        {
            totalSize = 0;
            return ;
        }
        file.seekg(0, std::ios::end);
        totalSize = file.tellg();
        file.seekg(0, std::ios::beg);
        file.close();
        headers["Content-Length"] =  intToString(totalSize);
        headers["Content-Type"] = "text/html";
    }
}

void HttpResponse::GeneratePageIndexing(std::string& fullpath,std::string& path, std::vector<std::string>& files)
{
    Page = DEFAULTINDEX;
    std::ofstream file(DEFAULTINDEX, std::ios::out | std::ios::trunc);
    if (!file)
    {
        std::cerr << "[Error] ... Could not open file for writing: " << path << std::endl;
        UpdateStatueCode(404);
        return;
    }
    std::ostringstream html;
    html << "<html><head><title>Index of " << fullpath << "</title></head><body>";
    html << "<h1>Index of " << fullpath << "</h1><ul>";
    for (size_t i = 0; i < files.size(); ++i)
        html << "<li><a href=\"" << ((path == "/") ?  "" : path ) + "/" + files[i] << "\">" << files[i] << "</a></li>";
    html << "</ul></body></html>";
    file << html.str();
    file.close();
}

void HttpResponse::HandleIndexing(std::string fullpath, std::string& path)
{
    struct stat pathStat;
    if (stat(fullpath.c_str(), &pathStat) != 0 || !S_ISDIR(pathStat.st_mode))
    {
        std::cerr << "[Error] : Path does not exist or is not a directory." << std::endl;
        UpdateStatueCode(500);
        return;
    }
    DIR* dir = opendir(fullpath.c_str());
    if (!dir) {
        std::cerr << "[Error] : Unable to open directory." << std::endl;
        UpdateStatueCode(500);
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
    GeneratePageIndexing(fullpath,path, files);
}

void HttpResponse::handleRequest(std::string& host, uint16_t port,HttpRequest & request)
{
    std::string uri = request.getUri();
    Route& route = request.getCurrentRoute();
    route.setRoot(PWD + route.getRoot());
    if(request.getUri().empty())
        uri = request.getUri() + "/";
    if (statusCode ==  204)
    {
        totalSize = 0;
        return ;
    }   
    else if (statusCode ==  200 || statusCode ==  201)
    {
        handleCookie(request);
        if (route.getPath() == uri)
        {
            if(!route.getDefaultFile().empty())
            {
                Page =  route.getRoot() + "/" + route.getDefaultFile();
                uri += (uri[uri.size() - 1] != '/') ? "/" +  route.getDefaultFile() : route.getDefaultFile();
            }
            else if(route.getAutoindex())
                HandleIndexing(route.getRoot(),route.getPath());
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
        CheckExistingInServer();
    }
    if (cgi)
        createEnvChar(request, uri, host, intToString(port));
}

std::string generateToken()
{
    const std::string charset = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";
    const size_t tokenLength = 32;
    std::string token;

    std::srand(static_cast<unsigned int>(std::time(0)));
    for (size_t i = 0; i < tokenLength; ++i) {
        size_t index = std::rand() % charset.size(); // Random index in charset
        token += charset[index];
    }
    return token;
}

void HttpResponse::handleCookie(HttpRequest & request)
{
    std::map<std::string, std::string>::iterator it = request.getheaders().find("Cookie");
    std::string token = generateToken();
    if (it ==  request.getheaders().end() &&  request.getUri() ==  SESSION)
    {
        headers["Set-Cookie"] = "session_id="+token + ";" + "path=" + SESSION;
        Cookies = "session_id="+token;
    }
    else if (it != request.getheaders().end())
        Cookies = it->second;
}

void HttpResponse::handleServerName(std::set<std::string>& serverNamesGetter, std::string hostrequest,std::string host)
{
    hostrequest = hostrequest.substr(0,hostrequest.find(":"));
    if (hostrequest == host)
        return;
    for (std::set<std::string>::iterator it = serverNamesGetter.begin(); it != serverNamesGetter.end(); ++it)
        if (*it == hostrequest)
            return ;
    UpdateStatueCode(400);
}

void HttpResponse::ResponseGenerating(HttpRequest & request,std::set<std::string>& serverNamesGetter, 
                    std::map<int, std::string> &errorPages, Status& status,std::string& host, uint16_t port, time_t currenttime)
{
    defaultErrors = errorPages;
    this->currenttime = currenttime;
    UpdateStatueCode(request.GetStatusCode());
    std::map<std::string, std::string>::iterator it = request.getheaders().find("Host");
    if  (it != request.getheaders().end())
        handleServerName(serverNamesGetter, it->second, host);
    else
        UpdateStatueCode(400);
    
    if(request.getCurrentRoute().getIsRedirection())
        handleRedirection(request.getCurrentRoute());
    else
        handleRequest(host,port, request);
    if (cgi)
    {
        int res = executeCGI();
        if (res == 1)
            UpdateStatueCode(500);
    }
    else
    {
        if(!request.getCurrentRoute().getIsRedirection())
            headers["Content-Type"] = getMimeType(Page);
        headers["Content-Length"] = intToString(totalSize);
    }
    headers["Date"] =  getCurrentTimeFormatted();
    headers["Server"] =  "WebServ1337";  
    headers["Connection"] = "close";
    status = SENDING_RESPONSE;
}


void HttpResponse::CheckExistingInServer()
{
    std::ifstream file(Page.c_str());
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        totalSize = file.tellg();
        file.seekg(0, std::ios::beg);
        file.close();
    }
    else
        UpdateStatueCode(404);
}















