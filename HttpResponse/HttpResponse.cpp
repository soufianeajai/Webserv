#include "HttpResponse.hpp"
#include "../Connection/Connection.hpp"

//1xx (Informational): The request was received, continuing process

//2xx (Successful): The request was successfully received,
//understood, and accepted

//3xx (Redirection): Further action needs to be taken in order to
//complete the request

//4xx (Client Error): The request contains bad syntax or cannot be
//fulfilled

//5xx (Server Error): The server failed to fulfill an apparently
//valid request

HttpResponse::HttpResponse():totaSize(0),offset(0),headerSended(false),cgi(false),PathCmd("")
{
}
std::string intToString(size_t number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

size_t HttpResponse::getOffset()
{
    return offset;
}

bool HttpResponse::getCgi() const
{
    return cgi;
}

pid_t HttpResponse::getPid() const
{
    return pid;
}

int HttpResponse::getpipe() const
{
    if (cgi)
        return pipefd[0];
    return -1;
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
        Page = route.getRoot() + route.getNewPathRedirection();
    }
    else
        UpdateStatueCode(404);
}

void HttpResponse::buildResponseBuffer(int clientSocketId, Status& status)
{
    ssize_t SentedBytes = 0;
    std::vector<uint8_t> response;
    
    try{
        if(!headerSended)
        {
            std::ostringstream oss;
            //std::cout <<"\nbuildResponseBuffer : (status : "<<status<<") "<<clientSocketId<<" "<< version << " " << statusCode << " " << reasonPhrase <<"\n";
            
            oss << version << " " << statusCode << " " << reasonPhrase << "\r\n";
            for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
                oss << it->first << ": " << it->second << "\r\n";
             oss << "\r\n";
            std::string responseStr = oss.str();
            response.insert(response.end(), responseStr.begin(), responseStr.end());
            // for(size_t i= 0;i < response.size();i++)
            //     std::cout << response[i];
            SentedBytes = send(clientSocketId, reinterpret_cast<char*>(response.data()), responseStr.size(), MSG_NOSIGNAL);
            if (SentedBytes < 0)
            { 

                std::cerr << "2 Send failed to client "<<clientSocketId << std::endl;
                status = DONE;
                return;
            }
            status = SENDING_RESPONSE;
            headerSended = true;
            response.clear();
        }
        if (cgi)
        {
            sendCgi(clientSocketId, status);
            return ;
        }
        size_t chunkSize = (totaSize < Connection::CHUNK_SIZE) ? totaSize : Connection::CHUNK_SIZE;
        std::ifstream file(Page.c_str(), std::ios::binary);
        if (!file.is_open() || totaSize == -1)
        {
            std::cerr << "no body for client :"<<clientSocketId << std::endl;
            status = DONE; 
            return;
        }
        file.seekg(offset, std::ios::beg); // go to position actual
        response.resize(chunkSize); // resize for chunck n 
        file.read(reinterpret_cast<char*>(response.data()), chunkSize);
        chunkSize = file.gcount(); // Get the number of bytes actually read
        offset += chunkSize;
        if (chunkSize > 0)
        {
            SentedBytes = send(clientSocketId, reinterpret_cast<char*>(response.data()), chunkSize, MSG_NOSIGNAL);
            if (SentedBytes < 0)
            { 
                std::cerr << "1 Send failed to client "<<clientSocketId << std::endl;
                file.close();
                status = DONE;  // handle error as needed
                return;
            }
            
        }
        if (chunkSize == 0 || offset >= static_cast<size_t>(totaSize))
        {
            file.close();            
            status = DONE;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Exception in buildResponseBuffer: " << e.what() << std::endl;
        status = DONE;
        // Handle any other standard exceptions that may occur
    }
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

    std::map<int, std::string>::iterator it = defaultErrors.find(statusCode);
    if (it != defaultErrors.end())
        Page = it->second;
    else
        Page = DEFAULTERROR;
    std::ifstream file(Page.c_str());
    if (!file.is_open())
    {
        std::cout << "\n-1\n";
        totaSize = -1; // this is last part we can do after check for error response , set it to -1 , then we check it in send response if equal to -1 so close connection no body for client !!
        return ;
    }
    file.seekg(0, std::ios::end);
    totaSize = file.tellg();
    file.seekg(0, std::ios::beg);
    file.close();
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

void HttpResponse::ResponseGenerating(HttpRequest & request, std::map<int, std::string> &errorPages, 
            int clientSocketId, Status& status,std::string& host, uint16_t port, time_t currenttime)
{
    std::cout << "detection : uri"<<request.getUri()<< " query: "<<request.getQuery()<<"\n";
     std::cout << "detection : host"<<host<< " port string: "<<intToString(port)<<"\n";
    defaultErrors = errorPages;
    std::string uri = request.getUri();
    Route& route = request.getCurrentRoute();
    version = request.getVersion();
    //cgi = !route.getCgiExtensions().empty();
    std::set<std::string> allowedMethods =route.getAllowedMethods();
    UpdateStatueCode(request.GetStatusCode());
    if (route.getRoot()[0] != '.')
        route.setRoot("." + route.getRoot());
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
    
    if(request.getUri().empty())
        uri = request.getUri() + "/";

    //std::cout << "get path : "<<route.getPath()<<"\nUri: "<<uri<<"\nget root: "<<route.getRoot()<<"\nis dir route: "<<route.isDirGetter()<<"\ndefualt file: "<<route.getDefaultFile()<<"\nauto index: "<<route.getAutoindex()<<"\n";
    
    if ((statusCode ==  200 || statusCode ==  201) && (allowedMethods.find("GET") != allowedMethods.end() ||  allowedMethods.find("POST") != allowedMethods.end())) // GET
    {
        if (route.getPath() == uri)
        {
            if(!route.getDefaultFile().empty())
            {
                Page =  route.getRoot() + "/" + route.getDefaultFile();
                uri += (uri[uri.size() - 1] != '/') ? "/" +  route.getDefaultFile() : route.getDefaultFile();
                std::cout << "\ndefault\n";
            }
            else if(route.getAutoindex())
            {
                std::cout << "\nindexing: "<<route.getRoot();
                HandleIndexing(route.getRoot(),request.getUri());
            }
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
                std::cout << "1 URI: \"" << uri << "\"" << std::endl;
                std::cout << "1 Path: \"" << route.getPath() << "\"" << std::endl;
                std::cout << "1 PAGE: \"" << Page << "\"" << std::endl;
                
                
            }
            else
                UpdateStatueCode(404);
        }
        checkIfCGI(request,Page, route.getCgiExtensions(), uri, host, intToString(port));
        std::cout << "2 URI: \"" << uri << "\"" << std::endl;
        std::cout << "2 Path: \"" << route.getPath() << "\"" << std::endl;
        std::cout << "2 PAGE: \"" << Page << "\"" << std::endl;  
    }
    else if (allowedMethods.find("DELETE") != allowedMethods.end() && statusCode ==  204)
    {
        std::cout << "[DELETE data]\n";
    }
    else
        UpdateStatueCode(404);
    
    if(route.getIsRedirection())
        handleRedirection(route);
    CheckExistingInServer();
    
    if (!route.getCgiExtensions().empty() && statusCode < 202)
    {
        std::cout << "\nwelcome to cgi :\n";
        
        if (cgi)
        {
            std::cout <<"exist cgi in this script\n";
            //cgi = ;
            int statue = executeCGI(currenttime);
            if (statue == 200)
                std::cout << "cgi runed\n";
            else
                UpdateStatueCode(statue);
        }
        else
            std::cout << "no exist\n";
    }
    headers["Content-Length"] =  intToString(totaSize);
    if (!cgi)
        headers["Content-Type"] =  getMimeType(Page);
    else
        headers["Content-Type"] = "text/html";
    headers["Date"] =  getCurrentTimeFormatted();
    headers["Server"] =  "WebServ 1337";  
    headers["Connection"] = "close";
    std::cout << "\npage->>>>>> : "<<Page<<" "<<uri<< "cgi ? : "<<cgi<<"\n";
    buildResponseBuffer(clientSocketId, status);
}


void HttpResponse::CheckExistingInServer()
{
    std::ifstream file(Page.c_str());
    if (file.is_open())
    {
        file.seekg(0, std::ios::end);
        totaSize = file.tellg();
        file.seekg(0, std::ios::beg);
        std::cout <<"\n!!!!!!!!!!!!!!!! file open is in my server : "<<Page.c_str()<<"\n";
        file.close();
    }
    else
    {
        UpdateStatueCode(404);
        std::cout <<"\n!!!!!!!!!!!!!!!! file not exist in my server : "<<Page.c_str()<<"\n";  
    }
}















