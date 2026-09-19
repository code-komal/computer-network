
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int client_socket;
    struct sockaddr_in server_addr;

    char file_data[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    int student_id;

    // Create TCP socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to server
    if (connect(client_socket,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(client_socket);
        exit(1);
    }

    printf("Connected to server.\n");

    // Read data.txt
    FILE *file = fopen("data.txt", "r");

    if (file == NULL)
    {
        perror("Unable to open data.txt");
        close(client_socket);
        exit(1);
    }

    memset(file_data, 0, sizeof(file_data));

    fread(file_data, sizeof(char), BUFFER_SIZE - 1, file);

    fclose(file);

    // Ask Student ID
    printf("Enter Student ID to search : ");
    scanf("%d", &student_id);

    // Convert ID to string
    sprintf(buffer, "%d", student_id);

    // Send Student ID
    write(client_socket, buffer, strlen(buffer) + 1);

    // Send file data
    write(client_socket, file_data, strlen(file_data) + 1);

    // Receive server response
    memset(buffer, 0, sizeof(buffer));

    read(client_socket, buffer, sizeof(buffer));

    printf("Server Response : %s\n", buffer);

    // Close socket
    close(client_socket);

    return 0;
}