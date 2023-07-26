#include <iostream>
#include <cstring>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <vector>

const int PORT = 8080;
const int backLog = 6;
const int buffSize = 1024;

int main()
{
    sockaddr_in serverAddr, clientAddr;
    int serverSocket, clientSocket;
    socklen_t clientAddrLen = sizeof(clientAddr);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("socket failed");
        return 1;
    }

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("binding failed");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, backLog) < 0) {
        perror("listening failed");
        close(serverSocket);
        return 1;
    }

    std::vector<int> clientSockets; // Store connected client sockets

    while (true)
    {
        fd_set readfds;
        FD_ZERO(&readfds);

        FD_SET(serverSocket, &readfds);

        int max_fd = serverSocket;
        for (const int& clientSock : clientSockets) {
            FD_SET(clientSock, &readfds);
            max_fd = std::max(max_fd, clientSock);
        }

        // Use select to monitor readfds for incoming data
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int readySockets = select(max_fd + 1, &readfds, nullptr, nullptr, &timeout);

        if (readySockets < 0) {
            perror("select failed");
            close(serverSocket);
            return 1;
        }

        if (FD_ISSET(serverSocket, &readfds)) {
            // Accept a new client
            clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket < 0) {
                perror("accept failed");
            } else {
                clientSockets.push_back(clientSocket);
                std::cout << "New client connected. Socket: " << clientSocket << std::endl;
                std::string MSG = "Welcome to the chat room!\r\n";
                send(clientSocket, MSG.c_str(), MSG.size(), 0);
            }
        }

        // Check data from each client socket
        for (auto it = clientSockets.begin(); it != clientSockets.end();) {
            int clientSock = *it;
            if (FD_ISSET(clientSock, &readfds)) {
                char buff[buffSize];
                int bytesReceived = recv(clientSock, buff, buffSize - 1, 0);
                if (bytesReceived <= 0) {
                    // Client disconnected
                    std::cout << "Client disconnected. Socket: " << clientSock << std::endl;
                    close(clientSock);
                    it = clientSockets.erase(it);
                } else {
                    buff[bytesReceived] = '\0';
                    std::cout << "Received from client " << clientSock << ": " << buff << std::endl;
                    // Broadcast the message to other clients
                    for (const int& otherClientSock : clientSockets) {
                        if (otherClientSock != clientSock) {
                            std::stringstream ss;
                            ss << "user " << clientSock << ": ";
                            std::string str = ss.str();
                            std::cout << str << std::endl;
                            str = str + buff;
                            send(otherClientSock, str.c_str(), bytesReceived + str.length(), 0);    
                        }
                    }
                    ++it;
                }
            } 
            else {
                ++it;
            }
        }
    }

    // Close all client sockets
    for (int clientSock : clientSockets) {
        close(clientSock);
    }

    close(serverSocket);
    return 0;
}

