#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 2048
#define INPUT_SIZE 100

void display_error(const char *message) {
    fprintf(stderr, "Error: %s (WSAGetLastError: %d)\n", message, WSAGetLastError());
}

int main() {
    WSADATA wsa;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE];
    char input[INPUT_SIZE];
    int recv_size;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        display_error("WSAStartup failed");
        return 1;
    }

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        display_error("Socket creation failed");
        WSACleanup();
        return 1;
    }

    // Setup server info
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        display_error("Connection failed");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected to Airport Server!\n\n");

    while (1) {
        // Receive menu or messages from server
        memset(buffer, 0, BUFFER_SIZE);
        recv_size = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        
        if (recv_size == SOCKET_ERROR) {
            display_error("Receive failed");
            break;
        }
        if (recv_size == 0) {
            printf("Server closed the connection.\n");
            break;
        }
        
        buffer[recv_size] = '\0';
        printf("%s", buffer);

        // Check for exit condition from server
        if (strstr(buffer, "Goodbye") || strstr(buffer, "exit")) {
            break;
        }

        // Get user input
        printf("> ");
        fflush(stdout);
        
        if (fgets(input, INPUT_SIZE, stdin) == NULL) {
            printf("Input error or EOF received.\n");
            break;
        }
        
        // Remove newline and check for empty input
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) {
            printf("Please enter a valid command.\n");
            continue;
        }

        // Send input to server
        if (send(sock, input, strlen(input), 0) == SOCKET_ERROR) {
            display_error("Send failed");
            break;
        }

        // Special case for exit command
        if (strcmp(input, "exit") == 0) {
            printf("Closing connection...\n");
            break;
        }
    }

    // Cleanup
    closesocket(sock);
    WSACleanup();
    return 0;
}