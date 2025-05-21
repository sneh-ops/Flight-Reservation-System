#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <unistd.h>

#define PORT 8080
#define MAX 1024

int main() {
    WSADATA wsaData;
    int sock;
    struct sockaddr_in serverAddr;
    char buffer[MAX];

    // Initialize Winsock
    int wsaErr = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaErr != 0) {
        printf("WSAStartup failed. Error Code: %d\n", wsaErr);
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Connection failed");
        return 1;
    }
    printf("Connected to server.\n");

    while (1) {
        printf("\nCommands:\nhello\nview\nexit\nEnter: ");
        fgets(buffer, MAX, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        send(sock, buffer, strlen(buffer), 0);

        if (strncmp(buffer, "exit", 4) == 0)
            break;

        bzero(buffer, MAX);
        recv(sock, buffer, sizeof(buffer), 0);
        printf("Server: %s\n", buffer);
    }

    // Cleanup Winsock
    closesocket(sock);
    WSACleanup();
    return 0;
}
