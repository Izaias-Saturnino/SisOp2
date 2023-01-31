#include <iostream>
#include <fstream>
#include <vector>

std::vector<std::string> get_file_list(std::string username) {
    std::vector<std::string> file_list;
    // code to get file list of a user directory
    return file_list;
}

void handle_file_synchronization(int sockfd, std::string username) {
    std::vector<std::string> server_file_list = get_file_list(username);

    while (true) {
        // Receive command
        char buffer[1024];
        int n = read(sockfd, buffer, 1023);
        if (n < 0) {
            std::cerr << "Error reading from socket" << std::endl;
            break;
        } else if (n == 0) {
            // Connection closed
            break;
        } else {
            buffer[n] = '\0';

            // Handle command
            std::string cmd(buffer);
            if (cmd == "SYNC") {
                // Send file list to client
                std::string file_list_str;
                for (auto& file : server_file_list) {
                    file_list_str += file + '\n';
                }
                write(sockfd, file_list_str.c_str(), file_list_str.size());
            } else if (cmd.substr(0, 4) == "PUT ") {
                // Handle PUT command
                std::string file_path = cmd.substr(4);
                std::ofstream file(username + "/" + file_path, std::ios::binary);
                if (file) {
                    // Receive file contents
                    while (true) {
                        n = read(sockfd, buffer, 1023);
                        if (n <= 0) break;
                        file.write(buffer, n);
                    }
                } else {
                    std::cerr << "Error creating file" << std::endl;
                }
            }
        }
    }
}
