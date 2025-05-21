#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <unistd.h>

#define PORT 8080
#define MAX 1024

void handleClient(int clientSocket);

int main() {
    WSADATA wsaData;
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size;

    // Initialize Winsock
    int wsaErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaErr != 0) {
        printf("WSAStartup failed. Error Code: %d\n", wsaErr);
        return 1;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket error");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    // Listen for connections
    if (listen(serverSocket, 5) < 0) {
        perror("Listen failed");
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept client connection
    while (1) {
        addr_size = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addr_size);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }
        printf("Client connected.\n");
        handleClient(clientSocket);
        closesocket(clientSocket);  // Updated for Windows socket closure
    }

    // Cleanup Winsock
    WSACleanup();
    return 0;
}

void handleClient(int clientSocket) {
    char buffer[MAX];

    while (1) {
        bzero(buffer, MAX);
        recv(clientSocket, buffer, sizeof(buffer), 0);

        if (strncmp(buffer, "exit", 4) == 0) {
            printf("Client disconnected.\n");
            break;
        } else if (strncmp(buffer, "hello", 5) == 0) {
            send(clientSocket, "Hello from server!", 19, 0);
        } else if (strncmp(buffer, "view", 4) == 0) {
            FILE *fp = fopen("flights.txt", "r");
            if (!fp) {
                send(clientSocket, "No flights available.\n", 23, 0);
            } else {
                char temp[MAX] = "";
                fread(temp, 1, MAX, fp);
                send(clientSocket, temp, strlen(temp), 0);
                fclose(fp);
            }
        } else {
            send(clientSocket, "Unknown command.\n", 17, 0);
        }
    }
}
