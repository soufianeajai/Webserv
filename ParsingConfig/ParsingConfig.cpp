#include "ParsingConfig.hpp"

int serverFlag = 0;

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
bool locationBlock(Server &server, std::ifstream &FILE, std::vector<std::string> locationPath)
{
    std::string str;
    Route route;

    if (locationPath.size() < 2)
        locationPath.push_back("/");
    else if (locationPath.size() > 2)
    {
        std::cout << "Error: Invalid location" << std::endl;
        exit(1);
    }
    route.setPath(locationPath[1]);
    // std::cout << route.getPath() << std::endl;
    while (getline(FILE, str))
    {
        if (!str.empty())
        {
            if (str.find("location") != std::string::npos)
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
                // std::cout << "end location------->" << std::endl;
                if (locationBlock(server, FILE, arr))  // Check if "server" was found in the recursive call
                    return true;  // Stop further execution if "server" was found
            }
            else if (str == "server")
            {

                FILE.seekg(-str.length() - 1, std::ios_base::cur); // Go back one line
                if (FILE.fail()) {
                    FILE.clear();
                    FILE.seekg(0, std::ios_base::beg); // If seek fails, go to beginning
                }
                server.addRoute(route);
                return true;  // Indicate that "server" was found to all recursive calls
            }
            else if (str.find("host") != std::string::npos || str.find("port") != std::string::npos 
                || str.find("server_names") != std::string::npos || str.find("server_root") != std::string::npos 
                || str.find("error_page") != std::string::npos || str.find("client_body_size") != std::string::npos)
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
                    std::stringstream ss(str);
                    std::string to;
                    std::vector<std::string> arr;
                    while (getline(ss, to, ' '))
                    {
                        if (!to.empty())
                            arr.push_back(to);
                    }

                    if (arr[0].find("methods") != std::string::npos)
                    {
                        if (arr.size() < 2 || arr.size() > 4)
                        {
                            std::cout << "Error: Invalid methods" << std::endl;
                            exit(1);
                        }
                        for (size_t i = 1; i < arr.size(); i++)
                        {
                            if (arr[i] != "GET" && arr[i] != "POST" && arr[i] != "DELETE")
                            {
                                std::cout << "Error: Invalid methods" << std::endl;
                                exit(1);
                            }
                            route.addAllowedMethod(arr[i]);
                        }
                        // std::set<std::string> methods = route.getAllowedMethods();
                        // for (std::set<std::string>::iterator it = methods.begin(); it != methods.end(); it++)
                        // {
                        //     std::cout << "|" << *it << "|";
                        // }
                        // std::cout << std::endl;
                    }
                    else if (arr[0].find("root") != std::string::npos)
                    {
                        if (arr.size() != 2)
                        {
                            std::cout << "Error: Invalid root" << std::endl;
                            exit(1);
                        }
                        route.setRoot(arr[1]);
                        // std::cout << "|" << route.getRoot() << "|" << std::endl;
                    }
                    else if (arr[0].find("default_file") != std::string::npos)
                    {
                        if (arr.size() != 2)
                        {
                            std::cout << "Error: Invalid default_file" << std::endl;
                            exit(1);
                        }
                        route.setDefaultFile(arr[1]);
                        // std::cout << "|" << route.getDefaultFile() << "|" << std::endl;
                    }
                    else if (arr[0].find("autoindex") != std::string::npos)
                    {
                        if (arr.size() != 2)
                        {
                            std::cout << "Error: Invalid autoindex" << std::endl;
                            exit(1);
                        }
                        if (arr[1] != "on" && arr[1] != "off")
                        {
                            std::cout << "Error: Invalid autoindex" << std::endl;
                            exit(1);
                        }
                        if (arr[1] == "on")
                            route.setAutoindex(true);
                        else if (arr[1] == "off")
                            route.setAutoindex(false);
                        else
                        {
                            std::cout << "Error: Invalid autoindex" << std::endl;
                            exit(1);
                        }
                        // if (route.getAutoindex())
                            // std::cout << "|" << "true" << "|" << std::endl;
                        // else
                            // std::cout << "|" << "false" << "|" << std::endl;
                    }
                    else if (arr[0].find("cgi_extension") != std::string::npos)
                    {
                        if (arr.size() < 2)
                        {
                            std::cout << "Error: Invalid cgi_extension" << std::endl;
                            exit(1);
                        }
                        for (size_t i = 1; i < arr.size(); i++)
                        {
                            route.addCgiExtension(arr[i]);
                        }
                        std::set<std::string> cgiExtensions = route.getCgiExtensions();
                        for (std::set<std::string>::iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); it++)
                        {
                            // std::cout << "|" << *it << "|";
                        }
                        // std::cout << std::endl;
                    }
                    else if (arr[0].find("redirect") != std::string::npos)
                    {
                        if (arr.size() != 3)
                        {
                            std::cout << "Error: Invalid redirect" << std::endl;
                            exit(1);
                        }
                        int i = 0;
                        while (arr[2][i])
                        {
                            if (!isdigit(arr[2][i]))
                            {
                                std::cout << "Error: Invalid redirect status code" << std::endl;
                                exit(1);
                            }
                            i++;
                        }
                        route.setIsRedirection(true);
                        route.setRedirectnewPath(arr[1]);
                        route.setRedirectCode(numberConversion(arr[2]));
                        // std::cout << "|" << server.getRedirectnewPath() << "|" << server.getRedirectPathOldPath() << "|" << server.getRedirectCode() << "|" << std::endl;
                    }
                    else
                    {
                        std::cout << "Error: Invalid config file" << std::endl;
                        exit(1);
                    }

            }
        }
    }
    server.addRoute(route);
    return false;  // Continue if "server" was not found
}
void ft_error(std::string err, std::ifstream& fd)
{
    fd.close();
    std::cerr << err << std::endl;
    exit (EXIT_FAILURE);
}
bool ParsingConfig::hostCheck(std::string host)
{
    std::stringstream ss(host);
    std::string to;
    std::vector<std::string> arr;
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
    if (str[str.length() - 1] != 'm' &&  str[str.length() - 1] != 'M'
       && str[str.length() - 1] != 'G' &&  str[str.length() - 1] != 'g')
        return false;
    return true;
}
ParsingConfig parsingConfig(const char *configFile)
{
    ParsingConfig parsingConfig;

    std::ifstream FILE(configFile);
    std::string str;

    while (str == "server" || getline(FILE, str))
    {

        if (str == "server") // Check if "server" was found in the recursive call
        {
            Server server;
            // std::cout << "server" << std::endl;
            while ((getline(FILE, str) && str != "server"))
            {
                serverFlag = 0;
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
                    if (arr[0].find("host") != std::string::npos)
                    {
                        if (arr.size() != 2 || !parsingConfig.hostCheck(arr[1]))
                            ft_error("invalid host", FILE);
                        server.hostSetter(arr[1]);
                    }
                    else if (arr[0].find("port") != std::string::npos)
                    {
                        if (arr.size() < 2 || arr[1] == " ")
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
                    else if (arr[0].find("server_names") != std::string::npos)
                    {
                        if (arr.size() < 2)
                            ft_error("Error: invalid server names", FILE);
                        for (size_t i = 1; i < arr.size(); i++)
                        {
                            server.serverNamesSetter(arr[i]);
                        }
                        std::vector<std::string> serverNames = server.serverNamesGetter();
                    }
                    else if (arr[0].find("server_root") != std::string::npos)
                    {
                        if (arr.size() != 2)
                            ft_error("Error: Invalid server_root", FILE);
                        server.serverRootSetter(arr[1]);
                    }
                    else if (arr[0].find("error_page") != std::string::npos)
                    {
                        if (arr.size() != 3)
                            ft_error("Error: Invalid error_page", FILE);
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
                    else if (arr[0].find("client_body_size") != std::string::npos)
                    {
                        
                        if (arr.size() != 2 || !parsingConfig.checkClientBodySize(arr[1]))
                        {
                            std::cout << "Error: Invalid client_body_size" << std::endl;
                            exit(1);
                        }
                        
                        server.clientMaxBodySizeSetter(numberConversion(arr[1]));
                        // std::cout << "|" << server.clientMaxBodySizeGetter() << "|" << std::endl;
                    }
                    else if (arr[0].find("location") != std::string::npos)
                    {

                        if (locationBlock(server, FILE, arr) && serverFlag == 1) // check dyal serverFlag ila kant chi location block flowel dyal server
                            break;
                    }
                    else
                    {
                        std::cout << "Error: Invalid config file" << std::endl;
                        exit(1);
                    }
                }
            }
            // std::cout << "-----------" << std::endl;
            // std::map<int, std::string> errorPages = server.errorPagesGetter();
            // for (std::map<int, std::string>::iterator it = errorPages.begin(); it != errorPages.end(); it++)
            // {
                // std::cout << "|" << it->first << "|" << it->second << "|" << std::endl;
            // }
            // std::cout << "-----------" << std::endl;
            parsingConfig.webServer.addServer(server);
        }    
    }
    FILE.close();
    return parsingConfig;
}
