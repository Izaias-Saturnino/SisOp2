#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

int main() {
    int sockfd, new_sockfd;
    socklen_t clilen;
    struct sockaddr_un serv_addr, cli_addr;

    // Create a socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error opening socket" << std::endl;
        return 1;
    }

    // Bind the socket to a local address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, "/tmp/dropbox.sock");
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while (true) {
        // Accept a new connection
        new_sockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (new_sockfd < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            return 1;
        }

        // Handle the connection
        while (true) {
            char buffer[1024];
            int n = read(new_sockfd, buffer, 1023);
            if (n < 0) {
                std::cerr << "Error reading from socket" << std::endl;
                break;
            } else if (n == 0) {
                // Connection closed
                break;
            } else {
                buffer[n] = '\0';
                // Handle the received data (e.g. store it in the file system)
            }
        }

        // Close the connection
        close(new_sockfd);
    }

    close(sockfd);
    return 0;
}
