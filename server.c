#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <stdbool.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_FLIGHTS 20
#define BUFFER_SIZE 2048
#define PORT 8080

typedef struct {
    int flight_no;
    char dest[30];
    char time[10];
    int economy;
    int business;
    int eco_price;
    int bus_price;
} Flight;

Flight flights[MAX_FLIGHTS] = {
    {101, "Mumbai", "10:00", 10, 5, 3000, 7000},
    {102, "Delhi", "14:00", 8, 3, 3500, 8000},
    {103, "Chennai", "18:30", 6, 4, 3200, 7500},
    {104, "Kolkata", "09:15", 9, 4, 3300, 7600},
    {105, "Odisha", "11:45", 7, 3, 3100, 7200},
    {106, "Kerala", "13:20", 8, 4, 3400, 7700},
    {107, "Bangalore", "15:50", 10, 5, 3600, 8000},
    {108, "Goa", "17:30", 12, 6, 3700, 8500},
    {109, "Kashmir", "06:45", 6, 2, 3800, 8800},
    {201, "New York", "20:00", 20, 10, 25000, 60000},
    {202, "Dubai", "21:30", 18, 8, 22000, 50000},
    {203, "Jakarta", "23:00", 16, 6, 21000, 48000},
    {204, "Nepal", "05:30", 14, 5, 15000, 35000},
    {205, "Singapore", "07:10", 17, 7, 23000, 53000},
    {206, "Japan", "08:25", 15, 6, 24000, 55000},
    {207, "South Korea", "19:00", 13, 5, 23500, 54500},
    {208, "Paris", "12:40", 19, 9, 26000, 60000},
    {209, "London", "16:55", 18, 8, 25500, 58000},
    {210, "Rome", "22:15", 17, 7, 24500, 57000}
};

void send_menu(SOCKET client_socket) {
    char menu[] = "\n--- Airport Reservation Menu ---\n"
                 "1. View Flights\n"
                 "2. Book Tickets\n"
                 "3. Exit\n"
                 "Enter choice: ";
    send(client_socket, menu, strlen(menu), 0);
}

void send_flights_list(SOCKET client_socket) {
    char buffer[BUFFER_SIZE] = "Available Flights:\n";
    
    for (int i = 0; i < MAX_FLIGHTS; i++) {
        char temp[256];
        sprintf(temp, "%d) Flight %d to %s at %s [Eco: %d seats @ Rs %d | Bus: %d seats @ Rs %d]\n",
                i+1, flights[i].flight_no, flights[i].dest, flights[i].time,
                flights[i].economy, flights[i].eco_price,
                flights[i].business, flights[i].bus_price);
        strcat(buffer, temp);
    }
    
    send(client_socket, buffer, strlen(buffer), 0);
}

bool book_tickets(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int fn, seats;
    char class_choice[20];
    
    // Get flight number
    send(client_socket, "Enter flight number: ", 21, 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    fn = atoi(buffer);
    
    // Find flight
    int index = -1;
    for (int i = 0; i < MAX_FLIGHTS; i++) {
        if (flights[i].flight_no == fn) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        send(client_socket, "Invalid flight number.\n", 23, 0);
        return false;
    }
    
    // Get class choice
    send(client_socket, "Class (Economy/Business): ", 26, 0);
    recv(client_socket, class_choice, BUFFER_SIZE, 0);
    class_choice[strcspn(class_choice, "\n")] = '\0';
    
    bool is_economy = (strcmp(class_choice, "Economy") == 0);
    
    // Get number of seats
    send(client_socket, "How many seats?: ", 17, 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    seats = atoi(buffer);
    
    // Process booking
    char msg[128];
    if (is_economy) {
        if (seats <= flights[index].economy) {
            flights[index].economy -= seats;
            sprintf(msg, "Booked %d Economy seat(s)! Total: Rs %d\n", 
                    seats, seats * flights[index].eco_price);
        } else {
            strcpy(msg, "Not enough Economy seats available.\n");
        }
    } else {
        if (seats <= flights[index].business) {
            flights[index].business -= seats;
            sprintf(msg, "Booked %d Business seat(s)! Total: Rs %d\n", 
                    seats, seats * flights[index].bus_price);
        } else {
            strcpy(msg, "Not enough Business seats available.\n");
        }
    }
    
    send(client_socket, msg, strlen(msg), 0);
    return true;
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;
    char buffer[BUFFER_SIZE];
    
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed. Error: %d\n", WSAGetLastError());
        return 1;
    }
    
    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    
    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);
    
    // Bind
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed. Error: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    
    // Listen
    listen(server_socket, 3);
    printf("Server running on port %d...\n", PORT);
    
    // Accept connection
    c = sizeof(struct sockaddr_in);
    client_socket = accept(server_socket, (struct sockaddr *)&client, &c);
    if (client_socket == INVALID_SOCKET) {
        printf("Accept failed. Error: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    
    printf("Client connected!\n");
    
    // Main server loop
    while (1) {
        send_menu(client_socket);
        
        // Get client choice
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        int choice = atoi(buffer);
        
        switch (choice) {
            case 1:
                send_flights_list(client_socket);
                break;
                
            case 2:
                book_tickets(client_socket);
                break;
                
            case 3:
                send(client_socket, "Thank you for using the reservation system. Goodbye!\n", 53, 0);
                closesocket(client_socket);
                closesocket(server_socket);
                WSACleanup();
                return 0;
                
            default:
                send(client_socket, "Invalid option. Please try again.\n", 34, 0);
        }
    }
    
    return 0;
}