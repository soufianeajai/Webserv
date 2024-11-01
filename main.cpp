#include "WebServers/Webservers.hpp"
#include "ServerSetup/Setup.hpp"
#include "ParsingConfig/ParsingConfig.hpp"

int main(int ac, char **av)
{
    ParsingConfig Config;
    if (ac != 2)
            return 1;
    Config = parsingConfig(av[1]);

    for (std::map<int, Server>::iterator it = Config.webServer.getServers().begin(); it != Config.webServer.getServers().end(); it++)
    {
        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "                      Server " << it->first << std::endl;
        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "Host: " << it->second.hostGetter() << std::endl;
        std::vector<int> ports = it->second.portGetter();
        for (size_t i = 0; i < ports.size(); i++)
        {
            std::cout << "Port: " << ports[i] << std::endl;
        }
        std::vector<std::string> serverNames = it->second.serverNamesGetter();
        for (size_t i = 0; i < serverNames.size(); i++)
        {
            std::cout << "Server Name: " << serverNames[i] << std::endl;
        }
        std::cout << "Server Root: " << it->second.serverRootGetter() << std::endl;
        std::map<int, std::string> errorPages = it->second.errorPagesGetter();
        for (std::map<int, std::string>::iterator it = errorPages.begin(); it != errorPages.end(); it++)
        {
            std::cout << "Error Page: " << it->first << " " << it->second << std::endl;
        }
        std::cout << "Client Max Body Size: " << it->second.clientMaxBodySizeGetter() << std::endl;
        std::cout << "Redirect New Path: " << it->second.getRedirectnewPath() << std::endl;
        std::cout << "Redirect Old Path: " << it->second.getRedirectPathOldPath() << std::endl;
        std::cout << "Redirect Code: " << it->second.getRedirectCode() << std::endl;
        std::cout << "------------------------------------------------" << std::endl;
        std::cout << "                          Routes: " << std::endl;
        std::cout << "------------------------------------------------" << std::endl;
        std::map<std::string, Route> routes = it->second.getRoutes();
        if (routes.empty()) {
            std::cout << "No routes found for this server." << std::endl;
        }
        for (std::map<std::string, Route>::iterator it = routes.begin(); it != routes.end(); it++)
        {
            std::cout << "*************************************************" << std::endl;
            std::cout << "Route: " << it->first << std::endl;
            std::cout << "Root: " << it->second.getRoot() << std::endl;
            std::cout << "Default File: " << it->second.getDefaultFile() << std::endl;
            if (it->second.getAutoindex())
                std::cout << "Autoindex: On" << std::endl;
            else
                std::cout << "Autoindex: Off" << std::endl;
            std::set<std::string> allowedMethods = it->second.getAllowedMethods();
            for (std::set<std::string>::iterator it = allowedMethods.begin(); it != allowedMethods.end(); it++)
            {
                std::cout << "Allowed Method: " << *it << std::endl;
            }
            std::set<std::string> cgiExtensions = it->second.getCgiExtensions();
            for (std::set<std::string>::iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); it++)
            {
                std::cout << "Cgi Extension: " << *it << std::endl;
            }
            std::cout << "*************************************************" << std::endl;
        }
    }
    // ServerSetup();
}