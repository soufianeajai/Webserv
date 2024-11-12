#include "HttpRequest.hpp"

State processMethod(std::string& myMethod, Route& route){
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

// bool isPrefix(std::string prefix, std::string uri)
// {
//     size_t prefixLength = prefix.length();
//     return ((uri.compare(0, prefixLength, prefix) == 0) /*&& (uri[prefixLength] == '/')*/);
// }

bool isDirectory(const std::string& path)
{
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) != 0)
    {
        perror("stat");  // Print an error if stat fails
        return false;    // Treat as non-directory if there's an error
    }
    return S_ISDIR(pathStat.st_mode);  // Check if it's a directory
}

void    HttpRequest::handleProcessUri_Method(std::map<std::string, Route>& routes, Route& myRoute)
{
    std::string normalize_url = uri;
    if (uri[uri.size()-1] != '/')
        normalize_url = uri + "/";
    std::map<std::string, Route>::iterator it = routes.find(normalize_url);
    bool found = false;
// exact matching
/*
    In NGINX, when using exact matching (=), 
    you generally avoid adding a trailing / in the location path if you want it to match a specific URL without subpaths.
*/
    if (it != routes.end()) {
        myRoute = it->second;
        found = true;
        CurrentRoute = it->second;
        //std::cout << "\n\n\n\n1found : "<<found<<"\n\n\n\n";
    }
    else{
        // check for prefix matching
        for (std::map<std::string, Route>::const_iterator it = routes.begin(); it != routes.end() && !found; ++it)
        {
            const std::string path = it->first;
            if ((uri.compare(0, path.length(), path) == 0))
            {
                myRoute = it->second;
                CurrentRoute = it->second;
                found = true;
               // std::cout << "\n\n\n\n2found : "<<found << " - "<<path<<"\n\n\n\n";// in case localhost:7070/uplodads/ is found !!
            }
        }
    }
    
    if (found)
        currentState = processMethod(this->getMethod(), myRoute);
    else
    {
        currentState = ERROR_INVALID_URI;
        //std::cout << "\n\n\n\n not found \n\n\n\n";
    }
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
    std::cout << "root " << myRoute.getRoot() <<  std::endl << "uri" << uri << std::endl;
    if (ft_rmdir(file_path.c_str()) == true)
        currentState = PROCESS_DONE;
    else
        currentState = ERROR_INVALID_METHOD;
}


void HttpRequest::saveDataToFile(std::string name, std::vector<uint8_t>& body)
{
    std::ofstream file(name.c_str(), std::ios::binary);
    if (file.is_open())
        file.write(reinterpret_cast<const char*>(body.data()), body.size());

    else {
        currentState = ERROR_BOUNDARY;
        return;
    }
    file.close();
}

void    HttpRequest::handleProcessPostData(){
    saveDataToFile("Posted_Data", body);
    currentState = PROCESS_DONE;
}

void    HttpRequest::handleProcessMultipart(std::string root){
    //upload the file in the current dir and the formfields in the server.
    std::string name = "boundary-filee";
    std::ofstream file(name.c_str());
    for(std::vector<boundaryPart>::iterator it = parts.begin(); it != parts.end(); it++){
        // for(std::map<std::string, std::string>::iterator itt = it->boundaryHeader.begin(); itt != it->boundaryHeader.end(); itt++)
        //     std::cout << itt->first << " " << itt->second << std::endl;
        if (it->isFile)
        {
            std::string name = root + uri + "/" + it->fileName;
            saveDataToFile(name, it->fileBody);
        }
        else
        {
            if (file.is_open()){
        //        std::cout << it->name << " " << it->value << std::endl;
                std::string toWrite = it->name + " " + it->value + "\n"; 
                file.write(toWrite.c_str(), toWrite.size());
            }

            else {
                currentState = ERROR_BOUNDARY;
                return;
            }
        }
    }
    file.close();
    currentState = PROCESS_DONE;
}

void    HttpRequest::handleProcessPost(){
    if (isChunked)
        currentState = PROCESS_CHUNKED_BODY;
    else if (isMultipart)
        currentState = PROCESS_MULTIPART_FORM_DATA;
    else if (contentLength)
        currentState = PROCESS_POST_DATA;
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
            saveDataToFile(filePath, body);
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
    // for(std::map<std::string, Route>::iterator it = routes.begin(); it != routes.end(); it++){
    //     std::cout << "route  : " << it->first << std::endl;
    //     std::cout << "pathLocation  : " << it->second.getPath() << std::endl;
    //     std::cout << "root  : " << it->second.getRoot() << std::endl << std::endl << std::endl;
    // }
    Route myRoute;
    currentState = PROCESS_URI;
    while (!errorOccured() && currentState != PROCESS_DONE) {
        switch (currentState)
        {
            case PROCESS_URI: handleProcessUri_Method(routes, myRoute); break;
            case PROCESS_DELETE: handleProcessDelete(myRoute); break;
            case PROCESS_POST: handleProcessPost(); break;
            case PROCESS_POST_DATA: handleProcessPostData(); break;
            case PROCESS_CHUNKED_BODY: handleProcessChunkedBody(myRoute.getRoot()); break;
            case PROCESS_MULTIPART_FORM_DATA: handleProcessMultipart(myRoute.getRoot()); break;
            case PROCESS_DONE: 
                break;
            default:
                break;
        }
    }
    
}

