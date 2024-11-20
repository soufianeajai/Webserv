
#include "WebServers/Webservers.hpp"
#include "serverSetup/ServerSetup.hpp"
#include "ParsingConfig/ParsingConfig.hpp"

int main(int ac, char **av)
{
    ParsingConfig Config;
    if (ac != 2)
            return 1;
    Config = parsingConfig(av[1]);
    std::vector<Server>server = Config.webServer.getServers();
    for (size_t i = 0; i < server.size(); i++)
    {
        std::cout << server[i].hostGetter() << std::endl;
        std::map<std::string, Route> routes = server[i].getRoutes();
        for (std::map<std::string, Route>::iterator it = routes.begin(); it != routes.end(); it++)
        {
            std::map<std::string, std::string> cgiExtensions = it->second.getCgiExtensions();
            for (std::map<std::string, std::string>::iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); it++)
            {
                std::cout << it->first << ": " << it->second << std::endl;
            }
        std::cout << "--------------------------------" << std::endl;
        }
    }

    // ServerSetup(Config);
}