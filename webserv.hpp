#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <vector>
#include <map>
#include <set>
#include <string>
#include <cstdint>
#include <iostream>

// Constant for default buffer size
const size_t DEFAULT_BUFFER_SIZE = 8192;

// Utility class for managing HTTP headers
class HeaderManager {
private:
    std::map<std::string, std::string> headers;

public:
    void addHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
    }

    const std::string& getHeader(const std::string& key) const {
        return headers.at(key);
    }

    void clear() {
        headers.clear();
    }

    const std::map<std::string, std::string>& getAllHeaders() const {
        return headers;
    }
};

// Buffer for I/O operations
class Buffer {
private:
    std::vector<uint8_t> data; // Vector to hold the buffered data
    size_t readPos = 0; // Current position in the buffer for reading
    size_t writePos = 0; // Current position in the buffer for writing

public:
    Buffer(size_t initialSize = DEFAULT_BUFFER_SIZE) : data(initialSize) {}

    bool write(const uint8_t* data, size_t len) {
        // Write data to the buffer and update write position
        // Implement write logic...
    }

    bool read(uint8_t* data, size_t len) {
        // Read data from the buffer and update read position
        // Implement read logic...
    }

    size_t available() const {
        return writePos - readPos;
    }

    void clear() {
        readPos = writePos = 0;
    }
};
// Connection Handler
class Connection {
private:
    int clientFd; // File descriptor for the client's socket connection
    Request request; // Current HTTP request being handled
    Response response; // HTTP response being prepared
    enum Status { READING, PROCESSING, WRITING, DONE } status; // Current status of the connection
    Buffer inputBuffer; // Buffer for incoming data
    Buffer outputBuffer; // Buffer for outgoing data

public:
    Connection(int fd) : clientFd(fd), status(READING) {}
// Handles reading data from the socket
    bool handleRead(){
    // Read data from the socket into inputBuffer
    // Then parse the request
    if (request.parse(inputBuffer)) {
        // Successfully parsed request
        status = PROCESSING;
    }
}
    bool handleWrite(); // Handles writing data to the socket
    void process(); // Processes the request and prepares the response
    bool isDone() const { return status == DONE; }
};


// HTTP Request Parser
class Request {
private:
    std::string method; // HTTP method
    std::string url; // Requested URL
    std::string version; // HTTP version
    HeaderManager headerManager; // Manages headers
    std::vector<uint8_t> body; // Raw body data
    bool isParsed; // Whether the request has been completely parsed

public:
    Request() : isParsed(false) {}

    bool parse(const Buffer& buffer); // Parses a raw HTTP request string
    const std::string& getMethod() const { return method; }
    const std::string& getUrl() const { return url; }
    const std::vector<uint8_t>& getBody() const { return body; }
    bool isValid() const; // Checks if the request is valid
};

// HTTP Response Builder
class Response {
private:
    std::string version; // HTTP version
    int statusCode; // HTTP status code
    std::string reasonPhrase; // Reason phrase
    HeaderManager headerManager; // Manages response headers
    std::vector<uint8_t> body; // Body of the response
    bool isReady = false; // Whether the response is fully prepared

public:
    Response() : statusCode(200) {}

    void setStatusCode(int code) { statusCode = code; }
    void addHeader(const std::string& key, const std::string& value) {
        headerManager.addHeader(key, value);
    }

    void setBody(const std::vector<uint8_t>& content) { body = content; }
    std::string build() const; // Builds the complete HTTP response string
};

// Route Class
class Route {
private:
    std::string path;
    std::set<std::string> allowedMethods; // Set for allowed HTTP methods
    std::string defaultFile;
    bool autoindex = false;
    std::string redirect;
    std::string uploadDir;
    std::set<std::string> cgiExtensions;

public:
    Route(const std::string& path) : path(path) {}
    bool matches(const std::string& uri) const;
    bool isMethodAllowed(const std::string& method) const;
    bool handleRequest(const Request& req, Response& res);
};

// Server Class
class Server {
private:
    std::string host;
    std::vector<int> ports;
    std::vector<std::string> serverNames; 
    std::string serverRoot;
    std::map<std::string, Route> routes;
    std::map<int, std::string> errorPages;
    size_t clientMaxBodySize;
    bool isDefault;

public:
    Server(const std::string& host, const std::vector<int>& ports) 
        : host(host), ports(ports) {}

    bool setup(); // Sets up the server
    bool listen(); // Starts listening for connections
    const std::string& getServerName() const { return serverNames.empty() ? host : serverNames[0]; }
    const std::map<std::string, Route>& getRoutes() const { return routes; }
    bool isDefaultServer() const { return isDefault; }
};


// Web Server Class
class WebServer {
private:
    Config config; // Configuration manager
    std::map<int, Connection*> connections; // Active connections

public:
    WebServer(const std::string& configFile) : config(configFile) {}

    bool initialize(); // Initializes the server
    void run(); // Main server loop
    void handleNewConnection(int serverFd); // Handles new connections
    void handleConnection(Connection* conn); // Handles existing connections
};

// Config Parser Class
class Config {
private:
    std::vector<Server> servers; // List of servers parsed from configuration

public:
    Config(const std::string& configFile) {
        parseConfig(configFile);
    }

    bool parseConfig(const std::string& configFile);
    const std::vector<Server>& getServers() const { return servers; }
};

#endif