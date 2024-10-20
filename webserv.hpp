#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <map>
#include <vector>
#include <set>

class WebServer {
private:
    Config config;
    std::vector<Server> servers;
    PollHandler pollHandler;
    std::map<int, Connection*> connections;

public:
    WebServer(const std::string& configFile);
    
    bool initialize();
    void run();
    void handleNewConnection(int serverFd);
    void handleConnection(Connection* conn);
};

// config Parser
class Config {
private:
    std::vector<Server> servers;
public:
    Config(const std::string& configFile);
    bool parseConfig(const std::string& configFile);
    const std::vector<Server>& getServers() const;
};


class Route {
private:
    std::string path;
    std::string allowedMethods[3];
    std::string defaultFile;
    bool autoindex = false;
    std::string redirect;
    std::string uploadDir;
    std::set<std::string> cgiExtensions;

public:
    Route(const std::string& path);
    bool matches(const std::string& uri) const;
    bool isMethodAllowed(const std::string& method) const;
    bool handleRequest(const Request& req, Response& res);
};

class Server {
private:
    std::string host;
    std::vector<int> ports;
    std::vector<std::string> serverNames; 
    std::string serverRoot;
    std::map<std::string, Route> routes;
    std::map<int, std::string> error_pages;
    size_t clientMaxBodySize;
    std::set<int> listenSockets; 
    std::map<int, struct sockaddr_in> serverAddrs;
    bool isDefault;

public:
    Server(const std::string& host, const std::vector<int>& ports);
    
    bool setup();
    bool listen();
    const std::string& getServerName() const;
    const std::map<std::string, Route>& getRoutes() const;
    bool isDefaultServer() const;
};


// Connection Handler
class Connection {
private:
    int clientFd; // File descriptor for the client's socket connection
    Request request; // The current HTTP request being handled for this connection
    Response response; // The HTTP response being prepared for this connection
    enum Status {READING, PROCESSING, WRITING, DONE}; // The current status of the connection (reading request, processing, writing response, or done)
    Status status; // The current status of the connection
    Buffer inputBuffer; // Buffer for incoming data from the client (used during reading)
    Buffer outputBuffer; // Buffer for outgoing data to the client (used during writing)

public:
    Connection(int fd); // Constructor initializes the connection with the client's file descriptor
    
    bool handleRead(); // Handles reading data from the client's socket into the input buffer
    bool handleWrite(); // Handles writing data from the output buffer to the client's socket
    void process(); // Processes the request and prepares the response based on the request
    bool isDone() const; // Returns whether the connection is finished (request processed and response sent)
};

// HTTP Request Parser
class Request {
private:
//start-line    request-line   = method <single space> url <single space> HTTP-version
    std::string method; // HTTP method of the request (e.g., GET, POST) status code (invalid method) = 501 (RFC 9112) 
    std::string url; // URL of the request (e.g., "/index.html") status code (valid but not found) = 404  / (Method Not Allowed in this route) 405 (RFC 9112) 
    std::string version; // HTTP version (e.g., "HTTP/1.1")
    // for invalid input, syntax errors ... 400 Bad Request
 //headers   
    std::map<std::string, std::string> headers; // Headers of the HTTP request (e.g., "Host", "Content-Length")
//empty line \r\n or \n  or /r means end of the headers
// If the method = POST then there is a body.
    std::vector<uint8_t> body; // Raw body data, could be binary
    bool isParsed; // Whether the request has been completely parsed
    std::string boundary; //Content-Type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW

public:
    Request(); // Default constructor
    
    bool parse(const std::string& raw); // Parses a raw HTTP request string and populates the attributes
    const std::string& getMethod() const; // Returns the HTTP method
    const std::string& getUri() const; // Returns the requested URI
    const std::string& getBody() const; // Returns the body of the request
    bool isValid() const; // Checks if the request is valid (e.g., properly formatted and complete)
};

// HTTP Response Builder
class Response {
private:
//start-line    
    std::string version; // HTTP version (e.g., "HTTP/1.1")
    int statusCode; // HTTP status code (e.g., 200 OK, 404 Not Found)
    std::string Reason Phrase //: OK (a textual explanation of the status code).
//headers   
    std::map<std::string, std::string> headers; // HTTP response headers (e.g., "Content-Type", "Content-Length")
//empty line \r\n or \n or \r means end of the headers
    std::vector<uint8_t> body; // Body of the response (e.g., the HTML content, binary ...)
    bool isReady = false; // Whether the response is fully prepared and ready to be sent

public:
    Response(); // Default constructor

    void setStatusCode(int code); // Sets the HTTP status code for the response
    void addHeader(const std::string& key, const std::string& value); // Adds a header to the response
    void setBody(const std::string& content); // Sets the body content of the response
    std::string build() const; // Builds the complete HTTP response string to be sent to the client
};

// Buffer for I/O operations
class Buffer {
private:
    std::vector<uint8_t> data; // Vector to hold the buffered data
    size_t readPos = 0; // Current position in the buffer for reading
    size_t writePos = 0; // Current position in the buffer for writing

public:
    Buffer(size_t initialSize = 8192); // Constructor initializes the buffer with a default size (8KB)

    bool write(const uint8_t* data, size_t len); // Writes data to the buffer
    bool read(uint8_t* data, size_t len); // Reads data from the buffer
    size_t available() const; // Returns the number of bytes available for reading
    void clear(); // Clears the buffer (resets read and write positions)
};
