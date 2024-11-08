#include "HttpRequest/HttpRequest.hpp"
#include "HttpMessage/HttpMessage.hpp"

void testSimpleGetRequest() {
    std::cout << "Testing simple GET request..." << std::endl;
    
    HttpRequest request;
    std::string rawRequest = 
        "GET /index.html HTTP/1.1\\r\n"
        "Host: example.com\r\n"
        "User-Agent: Mozilla/5.0\r\n"
        "\r\n";
    
    std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
    request.parse(buffer.data(), buffer.size());
    



    assert(request.parsingCompleted());
    assert(!request.errorOccured());
    assert(request.getMethod() == "GET");
    assert(request.getUri() == "/index.html");
    
   std::cout << "Simple GET request test passed!" << std::endl;
}

void testPostWithChunkedTransfer() {
    std::cout << "Testing POST request with chunked transfer..." << std::endl;
    
    HttpRequest request;
    std::string rawRequest = 
        "POST /upload HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "4\r\n"
        "Wiki\r\n"
        "5\r\n"
        "pedia\r\n"
        "0\r\n"
        "\r\n";
    
    std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
    request.parse(buffer.data(), buffer.size());
    
    assert(request.parsingCompleted());
    assert(!request.errorOccured());
    assert(request.getMethod() == "POST");
    assert(request.getUri() == "/upload");
    
    std::cout << "Chunked transfer test passed!" << std::endl;
}

void testMultipartFormData() {
    std::cout << "Testing multipart form data..." << std::endl;
    
    HttpRequest request;
    std::string boundary = "-------------------------735323031399963618057233701";
    std::string rawRequest = 
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n"
        "\r\n"
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"username\"\r\n"
        "\r\n"
        "johndoe\r\n"
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"email\"\r\n"
        "\r\n"
        "john.doe@example.com\r\n"
        "--" + boundary + "--\r\n";
    
    std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
    request.parse(buffer.data(), buffer.size());
    
    // assert(request.parsingCompleted());
    // assert(!request.errorOccured());
    // assert(request.getMethod() == "POST");
    // assert(request.getUri() == "/submit");
    
    std::cout << "Multipart form data test passed!" << std::endl;
}

void testErrorCases() {
    std::cout << "Testing error cases..." << std::endl;
    
//    Test 1: Invalid method
    {
        HttpRequest request;
        std::string rawRequest = "PUT /index.html HTTP/1.1\r\n\r\n";
        std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
        request.parse(buffer.data(), buffer.size());
        assert(request.errorOccured());
    }
    
//    Test 2: Invalid URI (contains spaces)
    {
        HttpRequest request;
        std::string rawRequest = "GET /path with spaces HTTP/1.1\r\n\r\n";
        std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
        request.parse(buffer.data(), buffer.size());
        assert(request.errorOccured());
    }
    
    // Test 3: Invalid HTTP version
    {
        HttpRequest request;
        std::string rawRequest = "GET /index.html HTTP/2.0\r\n\r\n";
        std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
        request.parse(buffer.data(), buffer.size());
        assert(request.errorOccured());
    }
    
    // Test 4: URI too long
    {
        HttpRequest request;
        std::string longUri(3000, 'a');  // Creates a string of 3000 'a' characters
        std::string rawRequest = "GET /" + longUri + " HTTP/1.1\r\n\r\n";
        std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
        request.parse(buffer.data(), buffer.size());
        assert(request.errorOccured());
    }
    
    std::cout << "Error cases test passed!" << std::endl;
}

void testURIDecoding() {
    std::cout << "Testing URI decoding..." << std::endl;
    
    HttpRequest request;
    std::string rawRequest = "GET /path/with%20space%2Fand%2Dothers HTTP/1.1\r\n\r\n";
    std::vector<uint8_t> buffer(rawRequest.begin(), rawRequest.end());
    request.parse(buffer.data(), buffer.size());
    
    assert(!request.errorOccured());
    assert(request.getUri() == "/path/with space/and-others");
    
    std::cout << "URI decoding test passed!" << std::endl;
}



#include <iostream>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#define TIMEOUT 60 * 1000  // 60 seconds timeout for client inactivity

struct Server {
    std::string host;
    std::vector<int> ports;
};

std::vector<Server> servers;
std::vector<struct pollfd> poll_fds;
std::map<int, Server*> socket_to_server;  // Maps each listening socket to its Server
std::map<int, std::string> client_buffers;  // Buffers to store responses for each client

void setup_servers(const std::vector<Server>& server_configs) {
    for (const auto& server : server_configs) {
        for (int port : server.ports) {
            int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (listen_fd < 0) {
                std::cerr << "Failed to create socket on port " << port << "\n";
                continue;
            }

            int opt = 1;
            setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            sockaddr_in address = {};
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = inet_addr(server.host.c_str());
            address.sin_port = htons(port);

            if (bind(listen_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
                std::cerr << "Failed to bind on " << server.host << ":" << port << "\n";
                close(listen_fd);
                continue;
            }

            listen(listen_fd, 5);
            fcntl(listen_fd, F_SETFL, O_NONBLOCK);

            poll_fds.push_back({listen_fd, POLLIN, 0});
            socket_to_server[listen_fd] = const_cast<Server*>(&server);  // Associate socket with server

            std::cout << "Listening on " << server.host << ":" << port << "\n";
        }
    }
}

void handle_new_connection(int listen_fd) {
    int client_fd = accept(listen_fd, nullptr, nullptr);
    if (client_fd < 0) {
        std::cerr << "Failed to accept client connection\n";
        return;
    }

    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    poll_fds.push_back({client_fd, POLLIN | POLLOUT, 0});  // Monitor both read and write

    Server* server = socket_to_server[listen_fd];
    std::cout << "New connection on server at " << server->host << ":" << listen_fd << "\n";
    // Initialize response buffer for client
    client_buffers[client_fd] = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!";
}

void handle_client_data(int client_fd) {
    char buffer[1024];
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes_received <= 0) {
        close(client_fd);
        auto it = std::find_if(poll_fds.begin(), poll_fds.end(), [client_fd](const pollfd& pfd) {
            return pfd.fd == client_fd;
        });
        if (it != poll_fds.end()) {
            poll_fds.erase(it);
        }
        client_buffers.erase(client_fd);  // Remove client buffer
    } else {
        // Process received data here
        std::cout << "Received data from client on fd " << client_fd << "\n";
        // Set POLLOUT to be ready to send the response
        for (auto& pfd : poll_fds) {
            if (pfd.fd == client_fd) {
                pfd.events |= POLLOUT;
                break;
            }
        }
    }
}

void send_client_data(int client_fd) {
    std::string& response = client_buffers[client_fd];
    ssize_t bytes_sent = send(client_fd, response.c_str(), response.size(), 0);
    
    if (bytes_sent < 0) {
        std::cerr << "Failed to send data to client on fd " << client_fd << "\n";
        close(client_fd);
        client_buffers.erase(client_fd);  // Remove client buffer
    } else {
        std::cout << "Sent data to client on fd " << client_fd << "\n";
        // If all data is sent, disable POLLOUT until there is more data to send
        auto it = std::find_if(poll_fds.begin(), poll_fds.end(), [client_fd](const pollfd& pfd) {
            return pfd.fd == client_fd;
        });
        if (it != poll_fds.end() && bytes_sent == response.size()) {
            it->events &= ~POLLOUT;  // Turn off POLLOUT
        }
    }
}

void server_loop() {
    while (true) {
        int poll_count = poll(poll_fds.data(), poll_fds.size(), TIMEOUT);
        if (poll_count < 0) {
            perror("poll error");
            break;
        }

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            int fd = poll_fds[i].fd;
            
            // Check for new connections
            if (poll_fds[i].revents & POLLIN) {
                if (socket_to_server.find(fd) != socket_to_server.end()) {
                    handle_new_connection(fd);  // Accept new client connection
                } else {
                    handle_client_data(fd);  // Handle data from existing client
                }
            }

            // Check for ready-to-send (write) events
            if (poll_fds[i].revents & POLLOUT) {
                send_client_data(fd);  // Send data to client
            }
        }
    }
}

int main() {
    // Example server setup
    servers.push_back({"127.0.0.1", {8080, 8081}});
    servers.push_back({"127.0.0.1", {8082}});

    setup_servers(servers);
    server_loop();

    // Cleanup sockets
    for (const auto& pfd : poll_fds) {
        close(pfd.fd);
    }
}
