#include "ParsingConfig.hpp"

int numberConversion(std::string &string)
{
    int number;
    std::stringstream ss(string);
    ss >> number;
    if (ss.fail()) {            // Check for conversion success
        std::cerr << "Conversion failed!" << std::endl;
        exit(1);
    } 
    return number;
}
void ft_error(std::string err, std::ifstream& fd)
{
    fd.close();
    std::cerr << err << std::endl;
    exit (EXIT_FAILURE);
}

bool ParsingConfig::containsOnlySpaces(std::string &str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (str[i] != ' ')
            return false;
    }
    return true;
}

bool locationBlock(Server &server, std::ifstream &FILE, std::vector<std::string> locationPath)
{
    std::string str;
    Route route;

    if (locationPath.size() < 2)
        locationPath.push_back("/");
    else if (locationPath.size() > 2)
        ft_error("Error: Invalid location", FILE);
    route.setPath(locationPath[1]);
    if (locationPath[1] != "/" 
        && (locationPath[1][locationPath[1].length() - 1] == '/' || locationPath[1][0] != '/'))
        ft_error("Error: Invalid Route " + locationPath[1], FILE);

    while (getline(FILE, str))
    {
        if (!str.empty())
        {
            std::stringstream ss(str);
            std::string to;
            std::vector<std::string> arr;
            while (getline(ss, to, ' '))
            {
                if (!to.empty())
                    arr.push_back(to);
            }
            if (!arr.size())
                break;
            if (arr[0] == "location:")
            {
                std::stringstream ss(str);
                std::string to;
                std::vector<std::string> arr;
                while (getline(ss, to, ' '))
                {
                    if (!to.empty())
                        arr.push_back(to);
                }
                server.addRoute(route);
                if (locationBlock(server, FILE, arr))
                    return true;
            }
            else if (arr[0] == "server")
            {

                FILE.seekg(-str.length() - 1, std::ios_base::cur); // Go back one line
                if (FILE.fail()) {
                    FILE.clear();
                    FILE.seekg(0, std::ios_base::beg); // If seek fails, go to beginning
                }
                server.addRoute(route);
                return true;  // Indicate that "server" was found to all recursive calls
            }
            else if (arr[0] == "host:" || arr[0] == "port:"
                || arr[0] == "server_names:" || arr[0] == "server_root:"
                || arr[0] == "error_page:" || arr[0] == "client_body_size:")
            {
                FILE.seekg(-str.length() - 1, std::ios_base::cur); // Go back one line
                if (FILE.fail()) {
                    FILE.clear();
                    FILE.seekg(0, std::ios_base::beg); // If seek fails, go to beginning
                }
                server.addRoute(route);
                return true;
            }
            else
            {
                    if (arr[0] == "methods:")
                    {
                        for (size_t i = 1; i < arr.size(); i++)
                        {
                            if (arr[i] != "GET" && arr[i] != "POST" && arr[i] != "DELETE")
                            {
                                std::cout << "Error: Invalid methods" << std::endl;
                                exit(1);
                            }
                            route.addAllowedMethod(arr[i]);
                        }
                    }
                    else if (arr[0] == "root:")
                    {
                        if (arr.size() != 2)
                            ft_error("Error: invalid root", FILE);
                        if (route.getRoot().empty())
                            route.setRoot(arr[1]);
                        if (route.getRoot() != "/" && (route.getRoot()[0] != '/' || route.getRoot()[route.getRoot().length() - 1] == '/'))
                            ft_error("Error: invalid root" + route.getRoot(), FILE);
                    }
                    else if (arr[0] == "default_file:")
                    {
                        if (route.getDefaultFile().empty())
                            route.setDefaultFile(arr[1]);
                    }
                    else if (arr[0] == "autoindex:")
                    {
                        if (arr.size() != 2 || (arr[1] != "on" && arr[1] != "off"))
                            ft_error("Error: invalid autoindex", FILE);
                        if (arr[1] == "on")
                            route.setAutoindex(true);
                        else if (arr[1] == "off")
                            route.setAutoindex(false);
                        else
                            ft_error("Error: invalid autoindex", FILE);
                    }
                    else if (arr[0] == "cgi_extension:")
                    {
                        for (size_t i = 1; i < arr.size(); i++)
                        {
                            route.addCgiExtension(arr[i]);
                        }
                    }
                    else if (arr[0] == "redirect:")
                    {
                        int i = 0;
                        while (arr[2][i])
                        {
                            if (!isdigit(arr[2][i]))
                                ft_error("Error: invalid redirect status code", FILE);
                            i++;
                        }
                        route.setIsRedirection(true);
                        if (route.getRedirectnewPath().empty())
                            route.setRedirectnewPath(arr[1]);
                        int code = numberConversion(arr[2]);
                        if (code != 301 && code != 302 && code != 303 
                            && code != 307 && code != 308)
                            ft_error("Error: invalid redirect status code", FILE);
                        route.setRedirectCode(code);
                    }
                    else if (arr[0] == "upload_dir:")
                    {
                        if (route.getUploadDir().empty())
                            route.setUploadDir(arr[1]);
                    }
                    else
                        ft_error("Error: " + arr[0], FILE);
            }
        }
    }
    server.addRoute(route);
    return false;  // Continue if "server" was not found
}

bool ParsingConfig::hostCheck(std::string host)
{
    std::stringstream ss(host);
    std::string to;
    std::vector<std::string> arr;
    int dotsCount = 0;
    for (size_t i = 0; i < host.length(); i++)
        if (host[i] == '.')
            dotsCount++;
    if (dotsCount != 3)
        return false;
    while (getline(ss, to, '.'))
    {
        if (!to.empty())
            arr.push_back(to);
    }
    if (arr.size() != 4)
        return false;
    for (size_t i = 0; i < arr.size(); i++)
    {
        if (arr[i].length() > 3 || numberConversion(arr[i]) > 255)
            return false;
        for (size_t j = 0; j < arr[i].length(); j++)
            if (arr[i][j] < '0' || arr[i][j] > '9')
                return false;
    }
    return true;
}
bool ParsingConfig::checkClientBodySize(std::string &str)
{
    for (size_t i = 0; i < str.length() - 1; i++)
    {
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    if (str[str.length() - 1] != 'm' &&  str[str.length() - 1] != 'M')
        return false;
    return true;
}

void checkDefaultServer(WebServer &webServer)
{
    std::vector<Server> servers = webServer.getServers();
    for (size_t i = 1; i < servers.size(); i++) {
        if (servers[0].hostGetter() == servers[i].hostGetter())
        {
            std::vector<int> portsDefault = servers[0].portGetter();
            std::vector<int> ports = servers[i].portGetter();
            for (size_t j = 0; j < portsDefault.size(); j++)
            {
                for (size_t k = 0; k < ports.size(); k++) {
                    if (portsDefault[j] == ports[k]) {
                        webServer.getServer(i).portEraser(k);
                        ports = webServer.getServers()[i].portGetter();
                        break;
                    }
                }
            }
                        
        }
    }    
}
void checkNecessary(WebServer &webserver, std::ifstream& FILE)
{
    std::vector<Server> servers = webserver.getServers();
    for (size_t i = 0; i < servers.size(); i++)
    {
        if (servers[i].hostGetter().empty())
            ft_error("Error: host not found", FILE);
        else if (servers[i].portGetter().empty())
            ft_error("Error: port not found", FILE);
        
    }
    
}
ParsingConfig parsingConfig(const char *configFile)
{
    ParsingConfig parsingConfig;

    std::ifstream FILE(configFile);
    if (FILE.fail())
    {
        std::cout << "Error: failed to open file" << std::endl;
        exit(1);
    }
    std::string str;

    while (str == "server" || getline(FILE, str))
    {
        if (str == "server") // Check if "server" was found in the recursive call
        {
            Server server;
            while (getline(FILE, str) && str != "server")
            {
                if (!str.empty() && !parsingConfig.containsOnlySpaces(str))
                {
                    std::stringstream ss(str);
                    std::string to;
                    std::vector<std::string> arr;
                    while (getline(ss, to, ' '))
                    {
                        if (!to.empty())
                            arr.push_back(to);
                    }
                    if (!arr.size())
                        break;
                    if (arr[0] == "host:")
                    {
                        if (arr.size() != 2 || !parsingConfig.hostCheck(arr[1]))
                            ft_error("invalid host", FILE);
                        if (server.hostGetter().empty())
                            server.hostSetter(arr[1]);
                    }
                    else if (arr[0] == "port:")
                    {
                        if (arr.size() < 2)
                            ft_error("Error: invalid port", FILE);
                        int port;
                        for (size_t i = 1; i < arr.size(); i++)
                        {
                            port = numberConversion(arr[i]);
                            if (port < 0)
                                ft_error("negative port", FILE);
                            server.portSetter(port);
                        }
                    }
                    else if (arr[0] == "server_names:")
                    {
                        for (size_t i = 1; i < arr.size(); i++)
                            server.serverNamesSetter(arr[i]);
                        std::vector<std::string> serverNames = server.serverNamesGetter();
                    }
                    else if (arr[0] == "server_root:")
                    {
                        if (server.serverRootGetter().empty())
                            server.serverRootSetter(arr[1]);
                    }
                    else if (arr[0] == "error_page:")
                    {
                        if (arr.size() != 3)
                            ft_error("Error: invalid error page", FILE);
                        int i = 0;
                        while (arr[1][i])
                        {
                            if (!isdigit(arr[1][i]))
                                ft_error("Error: Invalid error_page", FILE);
                            i++;
                        }
                        int ErrorCode = numberConversion(arr[1]);
                        if (ErrorCode != 400 && ErrorCode != 403 && ErrorCode != 404
                            && ErrorCode != 405 && ErrorCode != 500 && ErrorCode != 505)
                            ft_error("Error: invalid error page code", FILE);
                        server.errorPagesSetter(numberConversion(arr[1]), arr[2]);
                    }
                    else if (arr[0] == "client_body_size:")
                    {
                        if (arr.size() != 2 || !parsingConfig.checkClientBodySize(arr[1]))
                            ft_error("Error: invalid client body size", FILE);
                        server.clientMaxBodySizeSetter(numberConversion(arr[1]));
                    }
                    else if (arr[0] == "location:")
                    {
                        if (!locationBlock(server, FILE, arr))
                            break;
                    }
                    else
                        ft_error("Error: " + arr[0], FILE);
                }
            }
            parsingConfig.webServer.addServer(server);
        }    
    }
    checkNecessary(parsingConfig.webServer, FILE);
    FILE.close();
    checkDefaultServer(parsingConfig.webServer);
    return parsingConfig;
}
