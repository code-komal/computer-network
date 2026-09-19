
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_socket)
{
    char buffer[BUFFER_SIZE];
    char record[BUFFER_SIZE];
    int student_id;
    int found = 0;

    // Receive Student ID
    read(client_socket, buffer, sizeof(buffer));
    student_id = atoi(buffer);

    // Receive file data from client
    memset(buffer, 0, sizeof(buffer));
    read(client_socket, buffer, sizeof(buffer));

    // Search Student ID in received data
    char *line = strtok(buffer, "\n");

    while (line != NULL)
    {
        int id;

        sscanf(line, "%d", &id);

        if (id == student_id)
        {
            strcpy(record, line);
            found = 1;
            break;
        }

        line = strtok(NULL, "\n");
    }

    // Send response
    if (found)
    {
        char response[BUFFER_SIZE];

        snprintf(response, sizeof(response),
                 "Record Found : %s", record);

        write(client_socket, response, strlen(response) + 1);
    }
    else
    {
        char response[] = "Record Not Found";

        write(client_socket, response, strlen(response) + 1);
    }

    close(client_socket);
    exit(0);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    // Create TCP socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_socket);
        exit(1);
    }

    // Listen for clients
    if (listen(server_socket, 5) < 0)
    {
        perror("Listen failed");
        close(server_socket);
        exit(1);
    }

    printf("Server is running on port %d...\n", PORT);

    while (1)
    {
        // Accept client connection
        client_socket = accept(server_socket,
                                (struct sockaddr *)&client_addr,
                                &client_len);

        if (client_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("Client connected.\n");

        // Create child process
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("Fork failed");
            close(client_socket);
        }
        else if (pid == 0)
        {
            // Child process
            close(server_socket);

            handle_client(client_socket);
        }
        else
        {
            // Parent process
            close(client_socket);

            // Parent continues accepting clients
            waitpid(-1, NULL, WNOHANG);
        }
    }

    close(server_socket);

    return 0;
}