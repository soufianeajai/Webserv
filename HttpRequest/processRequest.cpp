#include "HttpRequest.hpp"
#include <ctime>
State processMethod(std::string& myMethod, Route& route){
    std::set<std::string> allowedmethods;
    allowedmethods = route.getAllowedMethods();
    std::set<std::string>::iterator Methodit = allowedmethods.find(myMethod);
    if (Methodit == allowedmethods.end())
        return ERROR_METHOD_NOT_ALLOWED;
    else
    {
        if (myMethod == "GET")
            return PROCESS_DONE;
        else
            return (myMethod == "POST" ? PROCESS_POST : PROCESS_DELETE);
    };
}

bool isPrefix(std::string prefix, std::string uri)
{
    size_t prefixLength = prefix.length();
    return ((uri.compare(0, prefixLength, prefix) == 0) && (uri.length() == prefixLength || uri[prefixLength] == '/'));
}

void HttpRequest::handleProcessUri_Method(std::map<std::string, Route>& routes)
{
    std::map<std::string, Route>::iterator it;
    bool found = false;
    size_t longestMatchLength = 0;
    if (uri[uri.size() - 1] == '/')
        uri = uri.substr(0, uri.length() - 1);
    
    it = routes.find(uri);
    if (it != routes.end())
    {
        found = true;
        CurrentRoute = it->second;
    } 
    else 
    {
        for (it = routes.begin(); it != routes.end(); it++) {
            if (isPrefix(it->first, uri)) {
                if (it->first.length() > longestMatchLength) {
                    CurrentRoute = it->second;
                    longestMatchLength = it->first.length();
                    found = true;
                }
            }
        }
        if (found != true){
            std::map<std::string, Route>::iterator it = routes.find("/");
            if (it != routes.end()){
                CurrentRoute = it->second;
                found = true;
            }
        }

    }
    if (found)
        currentState = processMethod(this->getMethod(), CurrentRoute);
    else
        currentState = ERROR_INVALID_URI;
}

bool ft_rmdir(const char *path)
{
    struct stat buffer;
    if (stat(path, &buffer) != 0)
        return false;
    if (S_ISDIR(buffer.st_mode)) {
        DIR *dir = opendir(path);
        if (dir == NULL)
            return false;
        struct dirent *entry;
        while ((entry = readdir(dir))) {
            if (std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0)
                continue;
            std::string full_path = std::string(path) + "/" + entry->d_name;
            if (!ft_rmdir(full_path.c_str())) {
                closedir(dir);
                return false;
            }
        }
        closedir(dir);
    if (remove(path) != 0)
        return false;   
    }
    else
        if (remove(path) != 0)
            return false;
    return true;
}


void    HttpRequest::handleProcessDelete(){
    struct stat buffer;
    std::string file_path = getPWDVariable() + CurrentRoute.getRoot() +  uri.erase(0, CurrentRoute.getPath().size());
    if (ft_rmdir(file_path.c_str()) == true)
        currentState = PROCESS_DONE;
    else if (stat(file_path.c_str(), &buffer) != 0)
        currentState = ERROR_NOT_FOUND;
    else
        currentState = ERROR_INTERNAL_ERROR;
}

void HttpRequest::saveDataToFile(std::string name, std::vector<uint8_t>& body)
{
    std::string newName = "." + this->getCurrentRoute().getRoot() + '/' + name;
    std::ofstream file(newName.c_str(), std::ios::binary);
    if (file.is_open())
        file.write(reinterpret_cast<const char*>(body.data()), body.size());
    else {
        currentState = ERROR_INTERNAL_ERROR;
        return;
    }
    file.close();
}

void    HttpRequest::handleProcessPostData(){
    std::map<std::string, std::string>::iterator it = headers.find("Content-Disposition");
    if (it != headers.end())
    {
        std::string contentDisposition = it->second;
        std::size_t filenamePos = contentDisposition.find("filename=\"");
        if (filenamePos != std::string::npos)
        {
            filenamePos += 10;
            std::size_t filenameEndPos = contentDisposition.find("\"", filenamePos);
            std::string filename = contentDisposition.substr(filenamePos, filenameEndPos - filenamePos);
            saveDataToFile(filename, body);
        }
    }
    else
        saveDataToFile("Posted_Data", body);
    currentState = PROCESS_DONE;
}

void    HttpRequest::handleProcessMultipart(){
    std::string name = "." + this->getCurrentRoute().getRoot() + "/dataBase";
    std::ofstream file(name.c_str(), std::ios::app);
    for(std::vector<boundaryPart>::iterator it = parts.begin(); it != parts.end(); it++){
        if (file.is_open()){
            std::string data = it->name + ":" + it->value + " "; 
            file.write(data.c_str(), data.size());
            
        }
        else {
            currentState = ERROR_INTERNAL_ERROR;
            return;
        }
    }
    file.write("\n", 1);
    file.close();
    currentState = PROCESS_DONE;
}

void    HttpRequest::handleProcessPost(){
    if (isMultipart)
        currentState = PROCESS_MULTIPART_FORM_DATA;
    else if (isChunked || contentLength)
        currentState = PROCESS_POST_DATA;
    else
        currentState = PROCESS_DONE;
}


void HttpRequest::process(std::map<std::string, Route>& routes){
    currentState = PROCESS_URI;
    while (!errorOccured() && currentState != PROCESS_DONE) {
        switch (currentState)
        {
            case PROCESS_URI: handleProcessUri_Method(routes); break;
            case PROCESS_DELETE: handleProcessDelete(); break;
            case PROCESS_POST: handleProcessPost(); break;
            case PROCESS_POST_DATA: handleProcessPostData(); break;
            case PROCESS_MULTIPART_FORM_DATA: handleProcessMultipart(); break;
            case PROCESS_DONE: 
                break;
            default:
                break;
        }
    }
}

