#include "ParsingConfig/ParsingConfig.hpp"

void ServerSetup(ParsingConfig &Config);
int main(int ac, char **av)
{
    ParsingConfig Config;
    if (ac != 2)
            return 1;
    Config = parsingConfig(av[1]);
    ServerSetup(Config);
}
