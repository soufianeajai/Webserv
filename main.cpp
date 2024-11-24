
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
    //     std::map<std::string, Route> routes = server[i].getRoutes();
    //     for (std::map<std::string, Route>::iterator it = routes.begin(); it != routes.end(); it++)
    //     {
    //         std::map<std::string, std::string> cgiExtensions = it->second.getCgiExtensions();
    //         for (std::map<std::string, std::string>::iterator it = cgiExtensions.begin(); it != cgiExtensions.end(); it++)
    //         {
    //             std::cout << it->first << ": " << it->second << std::endl;
    //         }
    //     std::cout << "--------------------------------" << std::endl;
    //     }
    // }
     ServerSetup(Config);
}

// #include <string>
// #include <map>
// #include <map>
// #include <set>
// #include <sys/types.h>
// #include <vector>
// #include <fstream>
// #include <sstream>
// #include <iostream>
// #include <sstream>
// #include <cstddef>
// #include <stdexcept>
// #include <cstring>
// #include <unistd.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <algorithm>
// #include <arpa/inet.h>
// #include <fcntl.h>
// #include <sys/stat.h>  
// #include <dirent.h>


// typedef unsigned char uint8_t;
// #include <iostream>
// #include <vector>
// #include <stdexcept>

// bool parseCGIOutput(const std::vector<uint8_t>& cgiOutput, std::vector<uint8_t>& headers, std::vector<uint8_t>& body)
// {
//     const uint8_t delimiter[] = {'\r', '\n', '\r', '\n'};
//     const size_t delimiterLength = 4;

//     for (size_t i = 0; i + delimiterLength <= cgiOutput.size(); ++i)
//     {
//         if (cgiOutput[i] == delimiter[0] &&
//             cgiOutput[i + 1] == delimiter[1] &&
//             cgiOutput[i + 2] == delimiter[2] &&
//             cgiOutput[i + 3] == delimiter[3]) {
//             size_t headerEnd = i + delimiterLength;
//             headers.assign(cgiOutput.begin(), cgiOutput.begin() + headerEnd);
//             if (headerEnd < cgiOutput.size())
//                 body.assign(cgiOutput.begin() + headerEnd, cgiOutput.end());
//             else
//                 body.clear();
//             return true;
//         }
//     }
//     return false;
// }

// int main() {
//     // Simulate CGI output as a vector of uint8_t
//     std::vector<uint8_t> cgiOutput;

//     // Add headers
//     const char* headersPart = "Content-Type: text/html\r\nContent-Length: 24\r\n\r\n";
//     cgiOutput.insert(cgiOutput.end(), headersPart, headersPart + strlen(headersPart));

//     // Add body
//     const char* bodyPart = "<html><body>Hello</body></html>\r\n\r\n";
//     cgiOutput.insert(cgiOutput.end(), bodyPart, bodyPart + strlen(bodyPart));

//     // Test case: Add extra delimiter at the end to simulate the issue
//     const char* extraDelimiter = "\r\n\r\n";
//     cgiOutput.insert(cgiOutput.end(), extraDelimiter, extraDelimiter + strlen(extraDelimiter));

//     // Containers for headers and body
//     std::vector<uint8_t> headers;
//     std::vector<uint8_t> body;

//     // Parse the CGI output
//     if (parseCGIOutput(cgiOutput, headers, body)) {
//         std::cout << "Headers:\n";
//         for (size_t i = 0; i < headers.size(); ++i) {
//             std::cout << static_cast<char>(headers[i]);
//         }

//         std::cout << "\nBody:\n";
//         for (size_t i = 0; i < body.size(); ++i) {
//             std::cout << static_cast<char>(body[i]);
//         }
//         std::cout << std::endl;
//     } else {
//         std::cout << "Failed to parse CGI output. No headers found.\n";
//     }

//     return 0;
// }
