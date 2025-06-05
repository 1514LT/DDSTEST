#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring> // For memset

#define PORT 9090
#define SERVER_IP "127.0.0.1" // Change if receiver is on a different machine
#define BUFFER_SIZE 1024 // 1KB buffer, changed from 1MB
#define GIGABYTE (1024LL * 1024LL * 1024LL) // 1GB

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    long long total_bytes_sent = 0;

    std::vector<char> buffer(BUFFER_SIZE, 'A'); // Fill buffer with 'A'

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "\n Socket creation error \n";
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr)); // Ensure serv_addr is zeroed

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cout << "\nInvalid address/ Address not supported \n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "\nConnection Failed \n";
        perror("connect");
        return -1;
    }
    std::cout << "Connected to receiver. Starting to send 1GB of data..." << std::endl;

    ssize_t bytes_sent_this_call;
    while (total_bytes_sent < GIGABYTE) {
        long long remaining_bytes = GIGABYTE - total_bytes_sent;
        long long bytes_to_send = std::min((long long)buffer.size(), remaining_bytes);
        
        bytes_sent_this_call = send(sock, buffer.data(), bytes_to_send, 0);
        
        if (bytes_sent_this_call < 0) {
            perror("send failed");
            break;
        }
        if (bytes_sent_this_call == 0) {
            std::cout << "Send returned 0, connection may be closed by peer." << std::endl;
            break;
        }
        total_bytes_sent += bytes_sent_this_call;
    }

    std::cout << "Sent " << total_bytes_sent << " bytes." << std::endl;
    if (total_bytes_sent == GIGABYTE) {
        std::cout << "Successfully sent 1GB of data." << std::endl;
    } else {
        std::cout << "Failed to send full 1GB. Sent " << total_bytes_sent << " bytes." << std::endl;
    }

    close(sock);
    return 0;
} 