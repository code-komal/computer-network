
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 3038
#define BUFFER_SIZE 1024

struct Request
{
    int student_id;
    char file_data[BUFFER_SIZE];
};

int main()
{
    int sock;
    struct sockaddr_in serv_addr;

    struct Request request;
    char buffer[BUFFER_SIZE];

    // 1. Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // 2. Define server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // 3. Connect to server
    if (connect(sock,
                (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        return 1;
    }

    // 4. Open data.txt
    FILE *file = fopen("data.txt", "r");

    if (file == NULL)
    {
        perror("Unable to open data.txt");
        close(sock);
        return 1;
    }

    // 5. Read file once
    memset(request.file_data, 0, BUFFER_SIZE);

    fread(request.file_data,
          sizeof(char),
          BUFFER_SIZE - 1,
          file);

    fclose(file);

    // 6. Keep asking for Student ID
    while (1)
    {
        printf("\nEnter Student ID to search (0 to exit): ");
        scanf("%d", &request.student_id);

        // Exit condition
        if (request.student_id == 0)
        {
            break;
        }

        // 7. Send request to server
        send(sock,
             &request,
             sizeof(request),
             0);

        // 8. Receive server response
        memset(buffer, 0, BUFFER_SIZE);

        int n = read(sock,
                     buffer,
                     BUFFER_SIZE - 1);

        if (n <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        buffer[n] = '\0';

        // 9. Display response
        printf("Server Response: %s\n", buffer);
    }

    // 10. Close connection
    close(sock);

    printf("Client disconnected.\n");

    return 0;
}