
// Roll: 24105142038 Service Type: B d = 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 3038
#define ROLL "24105142038"
#define BUFFER_SIZE 2048
#define MAX_NUMBERS 100

/* Process a Type B request and generate the response. */
void process_request(char *msg, char *response, size_t response_size)
{
    char *first_pipe = strchr(msg, '|');
    char *second_pipe;
    char *op;
    char *count_text;
    char *numbers;
    char *end;
    long count;
    long value;
    long long sum = 0;
    long long min = 0, max = 0;
    int i;

    if (first_pipe == NULL) {
        snprintf(response, response_size, "ERROR: Invalid request");
        return;
    }

    *first_pipe = '\0';
    op = msg;
    count_text = first_pipe + 1;

    second_pipe = strchr(count_text, '|');

    if (second_pipe == NULL) {
        snprintf(response, response_size, "ERROR: Invalid request");
        return;
    }

    *second_pipe = '\0';
    numbers = second_pipe + 1;

    if (strcmp(op, "SUM") != 0 &&
        strcmp(op, "MAX") != 0 &&
        strcmp(op, "MIN") != 0 &&
        strcmp(op, "AVG") != 0) {
        snprintf(response, response_size, "ERROR: Invalid operation");
        return;
    }

    errno = 0;
    count = strtol(count_text, &end, 10);

    if (errno != 0 || end == count_text || *end != '\0' ||
        count < 1 || count > MAX_NUMBERS) {
        snprintf(response, response_size, "ERROR: Invalid count");
        return;
    }

    for (i = 0; i < count; i++) {
        while (isspace((unsigned char)*numbers)) {
            numbers++;
        }

        errno = 0;
        value = strtol(numbers, &end, 10);

        if (errno != 0 || end == numbers ||
            value < INT_MIN || value > INT_MAX) {
            snprintf(response, response_size,
                     "ERROR: Invalid integer list");
            return;
        }

        if (i == 0) {
            min = max = value;
        } else {
            if (value < min) min = value;
            if (value > max) max = value;
        }

        sum += value;
        numbers = end;
    }

    /* Reject extra values or trailing non-whitespace characters. */
    while (isspace((unsigned char)*numbers)) {
        numbers++;
    }

    if (*numbers != '\0') {
        snprintf(response, response_size,
                 "ERROR: Number count mismatch");
        return;
    }

    if (strcmp(op, "SUM") == 0) {
        snprintf(response, response_size, "%lld", sum);
    } else if (strcmp(op, "MAX") == 0) {
        snprintf(response, response_size, "%lld", max);
    } else if (strcmp(op, "MIN") == 0) {
        snprintf(response, response_size, "%lld", min);
    } else {
        snprintf(response, response_size, "%.2f",
                 (double)sum / count);
    }
}

int main(void)
{
    int sockfd;
    int reuse = 1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    char client_ip[INET_ADDRSTRLEN];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
               &reuse, sizeof(reuse));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    printf("Server running on port %d\n", PORT);
    fflush(stdout);

    while (1) {
        ssize_t received;
        char *separator;
        size_t prefix_length;

        client_len = sizeof(client_addr);

        received = recvfrom(
            sockfd, buffer, sizeof(buffer) - 1, 0,
            (struct sockaddr *)&client_addr, &client_len
        );

        if (received < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[received] = '\0';

        inet_ntop(AF_INET, &client_addr.sin_addr,
                  client_ip, sizeof(client_ip));

        printf("\nClient: %s:%d\n",
               client_ip, ntohs(client_addr.sin_port));

        separator = strchr(buffer, '#');

        /* Validate the roll-number prefix before processing. */
        if (separator == NULL) {
            snprintf(response, sizeof(response),
                     "ERROR: Unauthorized client");
        } else {
            prefix_length = (size_t)(separator - buffer);

            if (prefix_length != strlen(ROLL) ||
                strncmp(buffer, ROLL, prefix_length) != 0) {
                snprintf(response, sizeof(response),
                         "ERROR: Unauthorized client");
            } else {
                char *request = separator + 1;

                printf("Received message: %s\n", request);

                if (strcmp(request, "EXIT") == 0) {
                    printf("Exit command received. Shutting down.\n");
                    break;
                }

                process_request(request, response,
                                sizeof(response));
            }
        }

        if (separator == NULL ||
            (size_t)(separator - buffer) != strlen(ROLL) ||
            strncmp(buffer, ROLL, strlen(ROLL)) != 0) {
            printf("Received message: %s\n", buffer);
        }

        if (sendto(sockfd, response, strlen(response), 0,
                   (struct sockaddr *)&client_addr,
                   client_len) < 0) {
            perror("sendto");
            continue;
        }

        printf("Sent response: %s\n", response);
        fflush(stdout);
    }

    close(sockfd);
    printf("Server stopped.\n");

    return 0;
}