#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <map>
#include <vector>
#include <set>

// Main WebServer class that ties everything together
class WebServer {
private:
    Config config; // The server configuration object
    std::vector<Server> servers; // List of servers (each representing a server block in the configuration)
    PollHandler pollHandler; // Handles polling for socket I/O events
    std::map<int, Connection*> connections; // Maps file descriptors to connection objects for handling requests

public:
    explicit WebServer(const std::string& configFile); // Constructor initializes the web server with a configuration file
    
    bool initialize(); // Initializes the web server (parses config, sets up servers, etc.)
    void run(); // Starts the main server loop, handling incoming requests
    void handleNewConnection(int serverFd); // Handles a new client connection on a server socket
    void handleConnection(Connection* conn); // Processes an existing connection (read/write)
};

// Configuration Parser
class Config {
private:
    std::vector<Server> servers; // List of server configurations parsed from the config file
    std::map<int, std::string> errorPages; // Maps HTTP error codes (e.g., 404, 500) to custom error page file paths

public:
    explicit Config(const std::string& configFile); // Constructor takes the path to the configuration file
    
    bool parseConfig(const std::string& configFile); // Parses the configuration file and populates servers and other settings
    const std::vector<Server>& getServers() const; // Returns the list of servers configured
    size_t getClientMaxBodySize() const; // Returns the maximum body size allowed for client requests
    std::string getErrorPage(int errorCode) const; // Returns the custom error page path for a given HTTP error code
};

// Server class to handle individual server instances
class Server {
private:
    std::string host; // The server's host address, usually specified in the config file
    std::vector<int> ports; // Ports on which this server will listen for incoming requests
    std::string serverName; // The name of the server, often used for virtual hosting
    std::map<std::string, Route> routes; // Routes associated with this server, mapping paths to handlers (e.g., "/images" to a file server)
    std::set<int> listenSockets; // File descriptors for the sockets listening for connections on different ports
    std::map<int, struct sockaddr_in> serverAddrs; // Maps socket file descriptors to their associated sockaddr_in structures for network addresses
    bool isDefault; // there is only one default server
    size_t clientMaxBodySize; // Maximum allowable size for a client's request body, specified in the config

public:
    Server(const std::string& host, const std::vector<int>& ports); // Constructor that sets the host and ports
    
    bool setup(); // Prepares the server's address structures and sockets
    bool listen(); // Puts the server's sockets into listening mode
    const std::string& getServerName() const; // Retrieves the server's name
    const std::map<std::string, Route>& getRoutes() const; // Returns the routes for the server
    bool isDefaultServer() const; // Checks if this server is the default server for handling unmatched requests
};

// Poll Handler for non-blocking I/O
class PollHandler {
private:
    std::vector<struct pollfd> fds; // Vector of poll file descriptors to monitor for I/O readiness
    std::map<int, Connection*> connections; // Maps file descriptors to connection objects for handling requests

public:
    PollHandler() = default; // Default constructor
    ~PollHandler(); // Destructor to clean up resources

    void addFd(int fd, short events); // Adds a file descriptor to be monitored by poll, specifying which events (read, write) to watch for
    void removeFd(int fd); // Removes a file descriptor from the poll set
    int poll(int timeout); // Calls poll with a timeout to wait for events on monitored file descriptors
    bool canRead(int fd) const; // Checks if the specified file descriptor is ready for reading
    bool canWrite(int fd) const; // Checks if the specified file descriptor is ready for writing
};

// Connection Handler
class Connection {
private:
    int clientFd; // File descriptor for the client's socket connection
    Request request; // The current HTTP request being handled for this connection
    Response response; // The HTTP response being prepared for this connection
    enum Status { READING, PROCESSING, WRITING, DONE }; // The current status of the connection (reading request, processing, writing response, or done)
    Status status; // The current status of the connection
    Buffer inputBuffer; // Buffer for incoming data from the client (used during reading)
    Buffer outputBuffer; // Buffer for outgoing data to the client (used during writing)

public:
    explicit Connection(int fd); // Constructor initializes the connection with the client's file descriptor
    
    bool handleRead(); // Handles reading data from the client's socket into the input buffer
    bool handleWrite(); // Handles writing data from the output buffer to the client's socket
    void process(); // Processes the request and prepares the response based on the request
    bool isDone() const; // Returns whether the connection is finished (request processed and response sent)
};

// HTTP Request Parser
class Request {
private:
    std::string method; // HTTP method of the request (e.g., GET, POST)
    std::string uri; // URI of the request (e.g., "/index.html")
    std::string version; // HTTP version (e.g., "HTTP/1.1")
    std::map<std::string, std::string> headers; // Headers of the HTTP request (e.g., "Host", "Content-Length")
    std::vector<char> body;  // Raw body data, could be binary
    bool isParsed; // Whether the request has been completely parsed
    size_t contentLength = 0; // Content length specified in the headers
    bool isChunked = false; // Whether the request uses chunked transfer encoding

public:
    Request() = default; // Default constructor
    
    bool parse(const std::string& raw); // Parses a raw HTTP request string and populates the attributes
    const std::string& getMethod() const; // Returns the HTTP method
    const std::string& getUri() const; // Returns the requested URI
    const std::string& getBody() const; // Returns the body of the request
    bool isValid() const; // Checks if the request is valid (e.g., properly formatted and complete)
};

// HTTP Response Builder
class Response {
private:
    int statusCode = 200; // HTTP status code (e.g., 200 OK, 404 Not Found)
    std::map<std::string, std::string> headers; // HTTP response headers (e.g., "Content-Type", "Content-Length")
    std::vector<char> body; // Body of the response (e.g., the HTML content, binary ...)
    bool isReady = false; // Whether the response is fully prepared and ready to be sent

public:
    Response() = default; // Default constructor

    void setStatusCode(int code); // Sets the HTTP status code for the response
    void addHeader(const std::string& key, const std::string& value); // Adds a header to the response
    void setBody(const std::string& content); // Sets the body content of the response
    std::string build() const; // Builds the complete HTTP response string to be sent to the client
};

// Route Handler
class Route {
private:
    std::string path; // The path for the route (e.g., "/images")
    std::set<std::string> allowedMethods; // Set of allowed HTTP methods (e.g., GET, POST)
    std::string root; // Root directory for serving static files for this route
    bool directoryListing = false; // Whether directory listing is allowed for this route
    std::string defaultFile; // Default file to serve if a directory is requested (e.g., "index.html")
    std::string redirect; // URL to redirect to if applicable
    std::string uploadDir; // Directory for handling file uploads
    std::set<std::string> cgiExtensions; // Set of file extensions that trigger CGI execution (e.g., ".php", ".py")

public:
    explicit Route(const std::string& path); // Constructor initializes the route with its path
    
    bool matches(const std::string& uri) const; // Checks if the given URI matches this route
    bool isMethodAllowed(const std::string& method) const; // Checks if the HTTP method is allowed for this route
    bool handleRequest(const Request& req, Response& res); // Handles the request and prepares the response for this route
};

// CGI Handler
class CGI {
private:
    std::string scriptPath; // Path to the CGI script to be executed
    std::map<std::string, std::string> env; // Environment variables for the CGI process
    int inputPipe[2]; // Pipe for passing input to the CGI script
    int outputPipe[2]; // Pipe for reading output from the CGI script
    pid_t pid; // Process ID of the CGI script

public:
    explicit CGI(const std::string& path); // Constructor initializes the CGI handler with the script path
    
    bool execute(const Request& req, Response& res); // Executes the CGI script, passing the request and building the response
    void setupEnvironment(const Request& req); // Sets up the CGI environment variables based on the request
    bool handleIO(const std::string& input, std::string& output); // Handles reading from and writing to the CGI process
};

// File Handler
class FileHandler {
private:
    std::string rootDir; // Root directory for serving static files

public:
    explicit FileHandler(const std::string& root); // Constructor initializes the file handler with the root directory

    bool serveFile(const std::string& path, Response& res); // Serves a static file based on the requested path
    bool listDirectory(const std::string& path, Response& res); // Lists the contents of a directory if directory listing is enabled
    bool handleUpload(const std::string& path, const std::string& content); // Handles file uploads to the specified path
};

// Buffer for I/O operations
class Buffer {
private:
    std::vector<char> data; // Vector to hold the buffered data
    size_t readPos = 0; // Current position in the buffer for reading
    size_t writePos = 0; // Current position in the buffer for writing

public:
    explicit Buffer(size_t initialSize = 8192); // Constructor initializes the buffer with a default size (8KB)

    bool write(const char* data, size_t len); // Writes data to the buffer
    bool read(char* data, size_t len); // Reads data from the buffer
    size_t available() const; // Returns the number of bytes available for reading
    void clear(); // Clears the buffer (resets read and write positions)
};



// Exception classes for error handling
class WebservException : public std::exception {
private:
    std::string message; // Message describing the exception

public:
    explicit WebservException(const std::string& msg) : message(msg) {} // Constructor sets the exception message
    
    const char* what() const noexcept override {
        return message.c_str(); // Returns the exception message
    }
};

// Utility class for common operations
class Utils {
public:
    static std::string getMimeType(const std::string& extension); // Returns the MIME type for a given file extension
    static bool isDirectory(const std::string& path); // Checks if a given path is a directory
    static bool fileExists(const std::string& path); // Checks if a file exists at the given path
    static std::string urlDecode(const std::string& encoded); // Decodes a URL-encoded string
    static void setNonBlocking(int fd); // Sets a socket to non-blocking mode
    static std::string generateErrorPage(int statusCode); // Generates an HTML error page for a given status code
};
#endif