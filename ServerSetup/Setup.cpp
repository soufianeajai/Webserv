#include "Setup.hpp"

#define MAX_CLIENTS 5
void ServerSetup()
{

    // creating socket
    /*
        - domain AF_INET: Specifies the communication domain or protocol family in this case IPv4
        - type SOCK_STREAM: 
            - Specifies the type of socket, which defines the communication semantics.
            in this case we use SOCK_STREAM For a reliable, connection-oriented byte stream, typically used with TCP.
        - protocol 0: Setting protocol to 0 allows the system to choose the default protocol that matches domain and type in this case it use TCP
     */
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
        std::cout << "The socket not opened\n";
    else
        std::cout << "sockect opened successfully\n";

    // set the socket option to reuse the address
    /*
        - The setsockopt() function allows you to modify a socket's behavior, enabling you to decide how it handles different network conditions
        - SOL_SOCKET: The level argument specifies the protocol level at which the option resides.
            in this case SOL_SOCKET is used to manipulate options at the socket level.
        - The SO_REUSEADDR socket option forces the kernel to reuse a local socket in TIME_WAIT state without waiting for its natural timeout to expire. 
            This is useful when a server needs to be restarted and the previous socket is still in the TIME_WAIT state, preventing the server from binding to the port.
        - opt: The value argument points to a buffer containing the value for the requested option.
        - sizeof(opt): this argement has diffrent roles according to the type of the option in this case it turn on the option and it option will start binding to the port
            because it might be a struct or a simple data type
    */
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        std::cerr << "Failed to set SO_REUSEADDR\n";
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // initilize the environment for the sockaddr struct
    /*
        - sin_family: Specifies the communication domain or protocol family in this case IPv4
        - sin_port: specifies the port
            - The htons function is used to convert a 16-bit integer (typically a port number) from host byte order to network byte order.
        - sin_addr.s_addr: is the ip address
            - in case u don't know the ip address the INADDR_ANY will fill in the IP address for you.
    */
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080); // htons() is used to convert a 16-bit integer from host byte order to network byte order
    serverAddress.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY allows the server to accept connections on any network interface
    

    // binding socket.
    /*
        - Associate a socket with an IP address and port number
            - its okay to cast the sockaddr_in to sockaddr* because both have the same size
    */
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
    {
        std::cout << "fail to bind local port\n";
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "successfully bind to local port\n";


    // Tell a socket to listen for incoming connections
    /*
        - listen() is used on the server side to make a socket ready to accept incoming connections.
        - The second parameter means how many pending connections you can have before the kernel starts rejecting new ones
    */
    if (listen(serverSocket, 5) < 0)
    {
        std::cout << "fail to listen for connection\n";
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "successfully listen for connection\n";
    /*
        The fcntl system call in Unix-based systems is used to manipulate file descriptors in this case we try to manupilate an open file discripotr to be non-blocking
        - first argument: The file descriptor you want to modify.
        - second argument: The command specifying what operation you want to perform (e.g., getting/setting flags).
        - third argument: The third argument is optional and varies depending on the command.
            in this case the flag O_NONBLOCK is what maek the file non-blocking.
    */
    fcntl(serverSocket, F_SETFL, O_NONBLOCK);


    /*
        Create an array of pollfd structures to monitor multiple socket connections:
        - fds[0] is reserved for the server socket that accepts new connections
        - The remaining slots (1 to MAX_CLIENTS) will store client connections
        - POLLIN event means we want to monitor for incoming data
    */
    struct pollfd fds[MAX_CLIENTS + 1]; // Array of pollfd structs
    fds[0].fd = serverSocket;
    fds[0].events = POLLIN; // Monitor for incoming connections


    int clientSocket;
    int numberOfFileDiscriptorsInFds = 1;
    int numberOfClients = 0;
    int timewaitIndex = 0;
    // an infinit loop to keep the server running
    while (1)
    {
        /*
            poll funciton allows a program to monitor multiple file descriptors
            to see if they are ready for reading, writing, or have encountered an error.
                - pointer to an array of struct pollfd which defines the file descriptors to be moitored and the events to check for each
                - this is the number of elements in the fds array
                - timeout in milliseconds
        */
        int poll_fd = poll(fds, numberOfFileDiscriptorsInFds, 5000);
        if (poll_fd == -1) {
            perror("poll");
            exit(1);
        } else if (poll_fd == 0) {
            printf("Timeout occurred! No data for 5 seconds.\n");
            timewaitIndex++;
            if (timewaitIndex == 3) {
                std::cout << "Server is shutting down\n";
                break;
            }
        }

        // detecing when a clients disconnect and closeing sokcects
        for (int i = 1; i < numberOfFileDiscriptorsInFds; i++) {
            // POLLHUP: Client closed connection (hung up)
            // POLLERR: Error occurred on socket
            // POLLNVAL: Invalid socket descriptor
            if (fds[i].revents & (POLLHUP | POLLERR | POLLNVAL)) {
                std::cout << "Client" << " disconnected -_-\n";
                // Connection closed by client
                close(fds[i].fd);
                // Remove this fd by shifting array
                for (int j = i; j < numberOfFileDiscriptorsInFds - 1; j++) {
                    fds[j] = fds[j + 1];
                }
                numberOfFileDiscriptorsInFds--;
                i--; // Adjust loop counter since we removed an element
            }
        }

        // Check if the server socket has an incoming connection 
        if (fds[0].revents & POLLIN) {
            // accepting connection request
            /*
                - The accept() function shall extract the first connection on the queue of pending connections, 
                    create a new socket with the same socket type protocol and address family as the specified socket, 
                    and allocate a new file descriptor for that socket.
            */
            clientSocket = accept(serverSocket, NULL, NULL);
            if (clientSocket < 0)
            {
                std::cout << "fail to accept connection\n";
                continue;
            }
            numberOfClients++;
            timewaitIndex = 0;
            // filling the fds struct with new pending connetion (client socket)
            if (numberOfFileDiscriptorsInFds < MAX_CLIENTS)
            {
                fds[numberOfFileDiscriptorsInFds].fd = clientSocket;
                fds[numberOfFileDiscriptorsInFds].events = POLLIN;
                numberOfFileDiscriptorsInFds++;
            } else {
                std::cout << "has reached the max elements" << std::endl;
                break;
            }
            // Set client socket to non-blocking mode
            fcntl(clientSocket, F_SETFL, O_NONBLOCK);
            // recieving data
            const char* httpResponse = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 13\r\n"
                "Connection: close\r\n"
                "\r\n"
                "Connection OK!";
            send(clientSocket, httpResponse, strlen(httpResponse), 0);
            std::cout << "sent response to client " << numberOfClients << std::endl;
        }
    }
    // Close the socket
    for (int i = 1; i < numberOfFileDiscriptorsInFds; i++) {
        if (fds[i].revents & (POLLHUP | POLLERR)) {
            close(fds[i].fd);
            fds[i].fd = -1;  // Mark as available
        }
    }
    close(serverSocket);
}