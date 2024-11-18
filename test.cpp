// #include <iostream>
// #include <cstdlib>
// #include <cstring>
// #include <unistd.h>
// #include <sys/wait.h>
// #include <fcntl.h>
// #include <vector>

// // Function to set up the environment for PHP-CGI
// void setupEnv(std::vector<char*>& envVars) {
//     // Set up required CGI environment variables
//      envVars.push_back(const_cast<char*>("REDIRECT_STATUS=200"));
//     envVars.push_back(const_cast<char*>("GATEWAY_INTERFACE=CGI/1.1"));
//     envVars.push_back(const_cast<char*>("REQUEST_METHOD=GET"));
//     envVars.push_back(const_cast<char*>("SCRIPT_NAME=/path/to/your/php/script.php"));
//     envVars.push_back(const_cast<char*>("SERVER_NAME=127.0.0.1"));
//     envVars.push_back(const_cast<char*>("SERVER_PORT=80"));
//     envVars.push_back(const_cast<char*>("SERVER_PROTOCOL=HTTP/1.1"));
//     envVars.push_back(const_cast<char*>("SERVER_SOFTWARE=MyWebServer/1.0"));
//     envVars.push_back(const_cast<char*>("QUERY_STRING="));  // No query string for this example
//     envVars.push_back(const_cast<char*>("CONTENT_TYPE="));  // No content type
//     envVars.push_back(const_cast<char*>("CONTENT_LENGTH="));  // No content length
//     envVars.push_back(NULL);  // Null-terminated list
// }

// int main() {
//     pid_t pid;
//     int status;

//     // Pipe for capturing PHP-CGI output
//     int pipefd[2];
//     if (pipe(pipefd) == -1) {
//         perror("pipe failed");
//         return 1;
//     }

//     pid = fork();
//     if (pid == -1) {
//         perror("fork failed");
//         return 1;
//     }

//     if (pid == 0) {
//         // Child process: execute php-cgi
//         dup2(pipefd[1], STDOUT_FILENO);  // Redirect stdout to pipe
//         close(pipefd[0]);  // Close read end of the pipe in child
//         close(pipefd[1]);  // Close write end of the pipe after redirect

//         // Set up environment variables
//         std::vector<char*> envVars;
//         setupEnv(envVars);

//         // Path to the php-cgi executable
//         const char* php_cgi_path = "/usr/bin/php-cgi";

//         // Arguments to pass to php-cgi: php-cgi executable and the script to execute
//         char* const args[] = { const_cast<char*>(php_cgi_path), const_cast<char*>("./your_script.php"), NULL };

//         // Execute the PHP-CGI process
//         if (execve(php_cgi_path, args, envVars.data()) == -1) {
//             perror("execve failed");
//             exit(1);
//         }
//     } else {
//         // Parent process: read from pipe
//         close(pipefd[1]);  // Close write end of the pipe in parent

//         char buffer[128];
//         ssize_t bytesRead;
//         while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
//             buffer[bytesRead] = '\0';  // Null-terminate the string
//             std::cout << buffer;  // Output the PHP-CGI result
//         }
//         close(pipefd[0]);  // Close read end of the pipe in parent

//         // Wait for child to finish
//         waitpid(pid, &status, 0);

//         // Check the exit status of the PHP-CGI process
//         if (WIFEXITED(status)) {
//             if (WEXITSTATUS(status) != 0) {
//                 std::cerr << "PHP-CGI script exited with error code: " << WEXITSTATUS(status) << std::endl;
//                 return 1;
//             }
//         } else {
//             std::cerr << "PHP-CGI script did not terminate normally." << std::endl;
//             return 1;
//         }
//     }

//     return 0;
// }
