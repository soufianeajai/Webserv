#include "ParsingConfig.hpp"

int serverFlag = 0;
bool locationBlock(Server &server, std::ifstream &FILE, std::vector<std::string> locationPath)
{
    // std::cout << "location ->" << std::endl;
    (void)server;
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
                // std::cout << "end location----->" << std::endl;
                serverFlag = 1;
                server.addRoute(route);
                return true;  // Indicate that "server" was found to all recursive calls
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
                    if (arr[0].find("root") != std::string::npos)
                    {
                        if (arr.size() != 2)
                        {
                            std::cout << "Error: Invalid root" << std::endl;
                            exit(1);
                        }
                        route.setRoot(arr[1]);
                        // std::cout << "|" << route.getRoot() << "|" << std::endl;
                    }
                    if (arr[0].find("default_file") != std::string::npos)
                    {
                        if (arr.size() != 2)
                        {
                            std::cout << "Error: Invalid default_file" << std::endl;
                            exit(1);
                        }
                        route.setDefaultFile(arr[1]);
                        // std::cout << "|" << route.getDefaultFile() << "|" << std::endl;
                    }
                    if (arr[0].find("autoindex") != std::string::npos)
                    {
                        if (arr.size() != 2)
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
                    if (arr[0].find("cgi_extension") != std::string::npos)
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
            }
        }
    }
    server.addRoute(route);
    return false;  // Continue if "server" was not found
}
ParsingConfig parsingConfig(const std::string& configFile)
{
    ParsingConfig parsingConfig;
    parsingConfig.configFile = configFile;
    std::ifstream FILE(parsingConfig.configFile);
    std::string str;
    int identifier = 0;
    while (str == "server" || getline(FILE, str))
    {
        if (str == "server" || serverFlag == 1)
        {
            Server server;
            // std::cout << "server" << std::endl;
            while (serverFlag == 1 || (getline(FILE, str) && str != "server"))
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
                    if (arr[0].find("host") != std::string::npos)
                    {
                        if (arr.size() != 2 )
                        {
                            std::cout << "Error: Invalid host" << std::endl;
                            exit(1);
                        }
                        server.hostSetter(arr[1]);
                        // std::cout << "|" <<server.hostGetter() << "|" << std::endl;
                    }
                    else if (arr[0].find("port") != std::string::npos)
                    {
                        if (arr.size() < 2 || arr[1] == " ")
                        {
                            std::cout << "Error: Invalid port" << std::endl;
                            exit(1);
                        }
                        for (size_t i = 1; i < arr.size(); i++)
                        {
                            server.portSetter(std::stoi(arr[i]));
                        }
                        std::vector<int> ports = server.portGetter();
                        for (size_t i = 0; i < ports.size(); i++)
                        {
                            // std::cout << "|" << ports[i] << "|";
                        }
                        // std::cout << std::endl;
                    }
                    else if (arr[0].find("server_names") != std::string::npos)
                    {
                        if (arr.size() < 2)
                        {
                            std::cout << "Error: Invalid server_names" << std::endl;
                            exit(1);
                        }
                        for (size_t i = 1; i < arr.size(); i++)
                        {
                            server.serverNamesSetter(arr[i]);
                        }
                        std::vector<std::string> serverNames = server.serverNamesGetter();
                        for (size_t i = 0; i < serverNames.size(); i++)
                        {
                            // std::cout << "|" << serverNames[i] << "|";
                        }
                        // std::cout << std::endl;
                    }
                    else if (arr[0].find("server_root") != std::string::npos)
                    {
                        if (arr.size() != 2)
                        {
                            std::cout << "Error: Invalid server_root" << std::endl;
                            exit(1);
                        }
                        server.serverRootSetter(arr[1]);
                        // std::cout << "|" << server.serverRootGetter() << "|" << std::endl;
                    }
                    else if (arr[0].find("error_page") != std::string::npos)
                    {
                        if (arr.size() != 3)
                        {
                            std::cout << "Error: Invalid error_page" << std::endl;
                            exit(1);
                        }
                        int i = 0;
                        while (arr[1][i])
                        {
                            if (!isdigit(arr[1][i]))
                            {
                                std::cout << "Error: Invalid error_page" << std::endl;
                                exit(1);
                            }
                            i++;
                        }
                        server.errorPagesSetter(std::stoi(arr[1]), arr[2]);
                    }
                    else if (arr[0].find("client_body_size") != std::string::npos)
                    {
                        if (arr.size() != 2 || !isdigit(arr[1][0]))
                        {
                            std::cout << "Error: Invalid client_body_size" << std::endl;
                            exit(1);
                        }
                        
                        server.clientMaxBodySizeSetter(std::stoi(arr[1]));
                        // std::cout << "|" << server.clientMaxBodySizeGetter() << "|" << std::endl;
                    }
                    else if (arr[0].find("redirect") != std::string::npos)
                    {
                        if (arr.size() != 4)
                        {
                            std::cout << "Error: Invalid redirect" << std::endl;
                            exit(1);
                        }
                        int i = 0;
                        while (arr[3][i])
                        {
                            if (!isdigit(arr[3][i]))
                            {
                                std::cout << "Error: Invalid redirect status code" << std::endl;
                                exit(1);
                            }
                            i++;
                        }
                        server.setRedirectnewPath(arr[1]);
                        server.setRedirectPathOldPath(arr[2]);
                        server.setRedirectCode(std::stoi(arr[3]));
                        // std::cout << "|" << server.getRedirectnewPath() << "|" << server.getRedirectPathOldPath() << "|" << server.getRedirectCode() << "|" << std::endl;
                    }
                    else if (arr[0].find("location") != std::string::npos)
                    {

                        if (locationBlock(server, FILE, arr))
                            break;
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
            parsingConfig.webServer.addServer(server, identifier++);
        }    
    }
    FILE.close();
    return parsingConfig;
}
