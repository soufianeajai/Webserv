#include "HttpRequest.hpp"

State processMethod(const std::string& myMethod, Route& route){
    std::set<std::string> allowedmethods;
    allowedmethods = route.getAllowedMethods();
    std::set<std::string>::iterator Methodit = allowedmethods.find(myMethod);
    if (Methodit == allowedmethods.end())
        return ERROR_INVALID_METHOD;
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
    return ((uri.compare(0, prefixLength, prefix) == 0) && (uri[prefixLength] == '/'));
}

void    HttpRequest::handleProcessUri_Method(std::map<std::string, Route>& routes, Route& myRoute)
{
    std::map<std::string, Route>::iterator it = routes.find(uri);
    bool found = false;
// exact matching
    if (it != routes.end()) {
        myRoute = it->second;
        found = true;
    }
    else{
        // check for prefix matching
        for (std::map<std::string, Route>::const_iterator it = routes.begin(); it != routes.end() && !found; ++it) {
            const std::string path = it->first;
            if (isPrefix(path, uri))
            {
                myRoute = it->second;
                found = true;
            }
    }
    }
    if (found)
        currentState = processMethod(this->getMethod(), myRoute);
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


void    HttpRequest::handleProcessDelete(Route& myRoute){
    std::string file_path = myRoute.getRoot() + uri;
    if (ft_rmdir(file_path.c_str()) == true)
        currentState = PROCESS_DONE;
    else
        currentState = ERROR_INVALID_METHOD;
}

void    HttpRequest::handleProcessMultipart(std::string root){
    //upload the file in the current dir and the formfields in the server.
    for(std::vector<boundaryPart>::iterator it = parts.begin(); it != parts.end(); it++){
        if (it->isFile)
        {
            std::string name = root + uri + "/" + it->fileName;
            std::ifstream file(name.c_str());
            if (file.is_open())
                it->fileBody.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
            else {
                currentState = ERROR_BOUNDARY;
                return;
            }
            file.close();
        }
    }
    currentState = PROCESS_DONE;
}
void    HttpRequest::handleProcessPost(){
    if (isChunked)
        currentState = PROCESS_CHUNKED_BODY;
    else if (isMultipart)
        currentState = PROCESS_MULTIPART_FORM_DATA;
}

void HttpRequest::handleProcessChunkedBody(std::string root) {
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
            std::string filePath = root + uri + "/" + filename;
            std::ifstream file(filePath.c_str(), std::ios::binary);
            if (file.is_open())
                body.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
            else{
                currentState = ERROR_BOUNDARY;
                return;
            }
            file.close();
        } else {
            currentState = ERROR_BOUNDARY;
            return;
        }
    } else {
        currentState = ERROR_BOUNDARY;
        return;
    }
    currentState = PROCESS_DONE;
}


void HttpRequest::process(std::map<std::string, Route>& routes){
    Route myRoute;
    currentState = PROCESS_URI;
    while (!errorOccured() && currentState != PROCESS_DONE) {
        switch (currentState)
        {
            case PROCESS_URI: handleProcessUri_Method(routes, myRoute); break;
            case PROCESS_DELETE: handleProcessDelete(myRoute); break;
            case PROCESS_POST: handleProcessPost(); break;
            case PROCESS_CHUNKED_BODY: handleProcessChunkedBody(myRoute.getRoot()); break;
            case PROCESS_MULTIPART_FORM_DATA: handleProcessMultipart(myRoute.getRoot()); break;
            case PROCESS_DONE: 
                break;
            default:
                break;
        }
    }
}


/*
- check for valid uri(locations) and method inside that location
- open files for upload, Post method
- configure a container to hold the posted data
- handle DELETE method.

*/