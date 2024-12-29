#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 8080
#define RESPONSE \
    "HTTP/1.1 200 OK\r\n" \
    "Content-Type: text/html; charset=utf-8\r\n\n\n" \

#define ERROR_RESPONSE \
    "HTTP/1.1 404 Not Found\r\n\n\n" \
    "Content-Type: text/html; charset=utf-8\r\n\n\n" \
    "\r\n" \
    "404 Not Found"

// FUNCTION THAT HANDLES HTML FILES
void send_html_response(int client_socket, const char* filename)
{
    FILE *file = fopen(filename, "r");

    if (!file) {
        write(client_socket, ERROR_RESPONSE, strlen(ERROR_RESPONSE));
        return;
    }

    // THIS GETS THE SIZE OF THE FILE CONTENT BY JUMPING TO THE END
    // OF THE FILE AND MEASURE HOW LONG IT IS, BUT THATS LIKE MOVING
    // THE START OF THE FILE TO THE END, SO REWIND SOLVES THAT.
    fseek(file, 0, SEEK_END);
    long int file_size = ftell(file);
    rewind(file);

    // CREATE A BUFFER FOR STORING THE FILE CONTENT
    char *file_content = malloc(file_size + 1);
    if (!file_content) {
        perror("MALLOC FAILED");
        fclose(file);
        return;
    }

    // GET THE CONTENT OF THE FILE
    fread(file_content, 1, file_size, file);
    fclose(file);
    file_content[file_size] = '\0';

    printf("File Content: %s\n", file_content);

    write(client_socket, RESPONSE, strlen(RESPONSE));
    write(client_socket, file_content, file_size);

    free(file_content);
}

int main ()
{
    int server_fd, accept_socket;
    struct sockaddr_in address;
    int addr_len = sizeof(address);
    char buffer[1024] = {0};
    size_t buffer_len = sizeof(buffer);

    // CREATING THE SOCKET
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // HANDLING SOCKET CREATION
    if (server_fd == 0) {
        perror("SOCKET FAILED");
        exit(1);
    }

    // BINDING THE SOCKET
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, addr_len) < 0) {
        perror("BIND FAILED");
        close(server_fd);
        exit(1);
    }

    // LISTENING
    if (listen(server_fd, 10) < 0) {
        perror("LISTEN FAILED");
        close(server_fd);
        exit(1);
    }

    printf("Server listeing on port %d\n", PORT);

    // ACCEPTING CONNECTIONS
    while (1) {
        if ((accept_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t *)&addr_len)) < 0) {
            perror("ACCEPT FAILED");
            continue;
        }

        // HANDLE CLIENT REQUEST (READ AND RESPOND)
        read(accept_socket, buffer, buffer_len);
        printf("Request received:\n%s\n", buffer);

        // SERVE THE HTML FILE
        // write(accept_socket, RESPONSE, strlen(RESPONSE));
        send_html_response(accept_socket, "index.html");

        // CLOSE THIS CLIENT CONNECTION
        close(accept_socket);
    }

    return 0;
}
