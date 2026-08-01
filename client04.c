#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char reg_no[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // Connect to server
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    while (1) {
        printf("\nEnter Registration Number (or type 'bye' to exit): ");
        fgets(reg_no, BUFFER_SIZE, stdin);
        reg_no[strcspn(reg_no, "\n")] = 0;

        send(sock, reg_no, strlen(reg_no), 0);

        // Exit condition
        if (strcmp(reg_no, "bye") == 0) {
            printf("Connection closed.\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("Server Response: %s\n", buffer);
    }

    close(sock);
    return 0;
}