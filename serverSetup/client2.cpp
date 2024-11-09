#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    const char* serverIP = "127.0.0.1";  
    const int serverPort = 8081;       

    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        return -1;
    }


    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverIP, &serverAddress.sin_addr) <= 0) {
        std::cerr << "Invalid address/Address not supported\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Connection to server failed\n";
        return -1;
    }

    const char* request =  "hello\r\n";
    for (int i = 0; i < 5; ++i) {
        if (send(sock, request, strlen(request), 0) < 0) {
            std::cerr << "Send failed\n";
            close(sock);
            return -1;
        }
        char buffer[1024];
		int bytesRead = recv(sock, buffer, sizeof(buffer), 0);
		if (bytesRead > 0)
		{
			std::cout << "Request received: " << buffer << std::endl;
			// hna fin ghadi tsendi response to the same client
		}
        memset(buffer, 0, sizeof(buffer));
        std::cout << "Request sent " << i + 1 << " times\n";
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        sleep(1);
    }
    
    close(sock);
    return 0;
}
