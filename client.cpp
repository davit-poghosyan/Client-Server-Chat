#include <iostream>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int PORT = 8080;
const int buffSize = 1024;
std::mutex m1, m2;


int main() {
    int clientSocket;
    sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket < 0) {
        perror("socket failed");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // Use the server IP address here

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("connection failed");
        close(clientSocket);
        return 1;
    }

    std::cout << "Connected to server." << std::endl;

    std::cout << "Enter a message to send to the server (type 'exit' to quit): ";
    char buffer[buffSize];

    while (true) {

       
        
        std::cin.getline(buffer, buffSize);

        if (std::string(buffer) == "exit") {
            break;
        }

        int sent = send(clientSocket, buffer, strlen(buffer), 0);
        if (sent < 0) {
            perror("failed to send");
            close(clientSocket);
            return 1;
        }

        int received = recv(clientSocket, buffer, buffSize - 1, 0);
        if (received < 0) {
            perror("failed to receive");
            close(clientSocket);
            return 1;
        }

        buffer[received] = '\0';
        std::cout << "Server response: " << buffer << std::endl;



    }

    close(clientSocket);
    return 0;
}

