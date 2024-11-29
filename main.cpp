#include "serverSetup/ServerSetup.hpp"
#include "ParsingConfig/ParsingConfig.hpp"

// void sighandler(ParsingConfig& config){
//     std::vector<Server>  servers = config.getServers();
//    for(std::vector<Server>::iterator it = servers.begin(); it != servers.end(); it++)
//         it->
// }

int main(int ac, char **av)
{
    ParsingConfig Config;
    if (ac != 2)
            return 1;
    Config = parsingConfig(av[1]);
    //signal(SIGINT, sighandler(Config));
//     std::vector<S, erver>server = Config.getServers();
//     for (size_t i = 0; i < server.size(); i++)
//     {
//         std::cout << server[i].hostGetter() << std::endl;
//         std::map<std::string, Route> routes = server[i].getRoutes();
//         for (std::map<std::string, Route>::iterator it = routes.begin(); it != routes.end(); it++)
//         {
//                 std::cout << it->second.getPath() << std::endl;
//                 std::cout << it->second.getAutoindex() << std::endl;
//         std::cout << "--------------------------------" << std::endl;
//         }
//     }
    ServerSetup(Config);
}
