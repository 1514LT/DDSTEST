#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <chrono>
#include <iomanip> // For std::fixed and std::setprecision

#define PORT 9090
#define BUFFER_SIZE 1024 // 1KB buffer, changed from 1MB
#define GIGABYTE (1024LL * 1024LL * 1024LL) // 1GB

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    long long total_bytes_received = 0;
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 9090
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 9090
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "Receiver listening on port " << PORT << std::endl;

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    std::cout << "Sender connected." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<char> buffer(BUFFER_SIZE);
    

    ssize_t bytes_read;
    while (total_bytes_received < GIGABYTE) {
        bytes_read = read(new_socket, buffer.data(), buffer.size());
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                std::cout << "Sender closed connection before sending 1GB." << std::endl;
            } else {
                perror("read error");
            }
            break;
        }
        total_bytes_received += bytes_read;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;

    std::cout << "Received " << total_bytes_received << " bytes." << std::endl;
    if (total_bytes_received == GIGABYTE) {
        std::cout << "Successfully received 1GB of data." << std::endl;
    } else {
        std::cout << "Did not receive full 1GB. Received " << total_bytes_received << " bytes." << std::endl;
    }
    std::cout << "Total time taken: " << std::fixed << std::setprecision(3) << elapsed_seconds.count() << " seconds." << std::endl;

    close(new_socket);
    close(server_fd);

    return 0;
} 