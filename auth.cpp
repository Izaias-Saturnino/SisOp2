#include <iostream>
#include <map>

std::map<std::string, std::string> users;

bool authenticate(int sockfd) {
    // Receive username and password
    char buffer[1024];
    int n = read(sockfd, buffer, 1023);
    if (n < 0) {
        std::cerr << "Error reading from socket" << std::endl;
        return false;
    }
    buffer[n] = '\0';

    // Extract username and password
    std::string auth_data(buffer);
    size_t separator_pos = auth_data.find(':');
    std::string username = auth_data.substr(0, separator_pos);
    std::string password = auth_data.substr(separator_pos + 1);

    // Check if the provided username and password match
    if (users.count(username) && users[username] == password) {
        // Send success message
        write(sockfd, "Authentication successful", sizeof("Authentication successful"));
        return true;
    } else {
        // Send failure message
        write(sockfd, "Authentication failed", sizeof("Authentication failed"));
        return false;
    }
}
