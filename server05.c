
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int count_words(char *str) {
    int count = 0, in_word = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n') {
            if (in_word == 0) {
                count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
    }

    return count;
}

int count_vowels(char *str) {
    int count = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        char ch = str[i];

        if (ch == 'a' || ch == 'e' || ch == 'i' ||
            ch == 'o' || ch == 'u' ||
            ch == 'A' || ch == 'E' || ch == 'I' ||
            ch == 'O' || ch == 'U') {
            count++;
        }
    }

    return count;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];

    // Create Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Initialize the value
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Write bind syntax here
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Write Listen Syntax here
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Write Accept function call
    socklen_t addrlen = sizeof(address);
    new_socket = accept(server_fd,
                        (struct sockaddr *)&address,
                        &addrlen);

    if (new_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        int valread = read(new_socket, buffer, BUFFER_SIZE);

        if (valread <= 0)
            break;

        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "bye") == 0)
            break;

        char command[20], text[BUFFER_SIZE];

        sscanf(buffer, "%[^|]|%[^\n]", command, text);

        if (strcmp(command, "ANALYZE") == 0) {

            int chars = strlen(text);
            int words = count_words(text);
            int vowels = count_vowels(text);

            char response[BUFFER_SIZE];

            sprintf(response,
                    "Chars = %d, Words = %d, Vowels = %d",
                    chars, words, vowels);

            send(new_socket, response, strlen(response), 0);

        } else {

            char *msg = "Invalid Command";

            send(new_socket, msg, strlen(msg), 0);
        }
    }

    close(new_socket);
    close(server_fd);

    return 0;
}