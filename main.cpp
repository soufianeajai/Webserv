
#include "WebServers/Webservers.hpp"
#include "serverSetup/ServerSetup.hpp"
#include "ParsingConfig/ParsingConfig.hpp"

int main(int ac, char **av)
{
    ParsingConfig Config;
    if (ac != 2)
            return 1;
    Config = parsingConfig(av[1]);
    // std::vector<Server>server = Config.webServer.getServers();
    // for (size_t i = 0; i < server.size(); i++)
    // {
    //     std::cout << server[i].hostGetter() << std::endl;
    //     std::cout << server[i].clientMaxBodySizeGetter() << std::endl;
    //     std::cout << server[i].serverRootGetter() << std::endl;
    //     std::cout << server[i].getIpaddress() << std::endl;
    //     std::cout << server[i].portGetter()[0] << std::endl;
    //     std::cout << server[i].serverNamesGetter()[0] << std::endl;
    //     std::map<int, std::string> errorPages = server[i].errorPagesGetter();
    //     for (std::map<int, std::string>::iterator it = errorPages.begin(); it != errorPages.end(); it++)
    //     {
    //         std::cout << it->first << " " << it->second << std::endl;
    //     }
    //     std::map<std::string, Route> routes = server[i].getRoutes();
    //     for (std::map<std::string, Route>::iterator it = routes.begin(); it != routes.end(); it++)
    //     {
    //         std::cout << it->first << " " << it->second.getPath() << std::endl;
    //         std::set<std::string> methods = it->second.getAllowedMethods();
    //         for (std::set<std::string>::iterator it = methods.begin(); it != methods.end(); it++)
    //         {
    //             std::cout << *it << std::endl;
    //         }
    //         std::cout << it->second.getRoot() << std::endl;
    //         std::cout << it->second.getDefaultFile() << std::endl;
    //         std::cout << "hahowa" << it->second.getAutoindex() << std::endl;
    //         std::cout << it->second.getIsRedirection() << std::endl;
    //         std::cout << it->second.getRedirectnewPath() << std::endl;
    //         std::set<std::string> cgiExtensions = it->second.getCgiExtensions();
    //         for (std::set<std::string>::iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); it++)
    //         {
    //             std::cout << *it << std::endl;
    //         }
    //     std::cout << "--------------------------------" << std::endl;
    //     }
    // }

    ServerSetup(Config);
}