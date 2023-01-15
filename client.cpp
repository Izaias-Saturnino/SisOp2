#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

int main() {
    int sockfd;
    struct sockaddr_un serv_addr;

    // Create a socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error opening socket" << std::endl;
        return 1;
    }

    // Connect the socket to the server
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, "/tmp/dropbox.sock");
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        return 1;
    }

    // Send data
    char buffer[] = "This is the message. It will be repeated.";
    int n = write(sockfd, buffer, sizeof(buffer));
    if (n < 0) {
        std::cerr << "Error writing to socket" << std::endl;
        return 1;
    }

    // Receive data
    memset(buffer, 0, sizeof(buffer));
    n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0) {
        std::cerr << "Error reading from socket" << std::endl;
        return 1;
    }
    buffer[n] = '\0';
    // Handle the received data (e.g. display it to the user)

    // Close the socket
    close(sockfd);

    return 0;
}
