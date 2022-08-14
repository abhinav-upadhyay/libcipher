#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define HTTP_PORT 80
#define MAX_GET_CMD_LEN 1024
#define BUFFER_SIZE 255
static char get_command[MAX_GET_CMD_LEN];
static char buffer[BUFFER_SIZE + 1];


/**
 * @brief Accept a well formed URL and return the hostname and path
 * This function modifies the uri itself.
 * 
 * @param uri 
 * @param host 
 * @param path 
 * @return 0 if success, -1 if error
 */
int
parse_url(char *uri, char **host, char **path)
{
    char *hostname_pos = strstr(uri, "://");
    if (hostname_pos == NULL) {
        fprintf(stderr, "Invalid URL\n");
        return -1;
    }
    hostname_pos += 3;
    *host = hostname_pos;
    char *path_pos = strchr(hostname_pos, '/');
    if (path_pos == NULL) {
        *path = "/";
    } else {
        *path_pos = 0;
        *path = path_pos + 1;
    }
    return 0;
}

/**
 * @brief Format and send an HTTP GET command. The return value be 0 on success and
 * -1 on error, with errno set appropriately. The caller must retrieve the response
 * 
 * @param client_connection 
 * @param path 
 * @param host 
 * @return int 
 */
static int
http_get(int client_connection, const char *path, const char *host)
{
    sprintf(get_command, "GET %s HTTP/1.1\r\n", path);
    if (send(client_connection, get_command, strlen(get_command), 0) == -1) {
        perror("Error sending GET command");
        return -1;
    }
    sprintf(get_command, "Host: %s\r\n", host);
    if (send(client_connection, get_command, strlen(get_command), 0) == -1) {
        perror("Error sending host header");
        return -1;
    }
    sprintf(get_command, "Connection: close\r\n\r\n");
    if (send(client_connection, get_command, strlen(get_command), 0) == -1) {
        perror("Error sending connection header");
        return -1;
    }
    return 0;
}

static void
display_result(int connection)
{
    int received = 0;
    while ((received = recv(connection, buffer, BUFFER_SIZE, 0 )) > 0) {
        buffer[received] = 0;
        printf("%s", buffer);
    }
    printf("\n");
}



int
main(int argc, char **argv)
{
    char *host, *path;
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <url>\n", argv[0]);
        return 1;
    }

    if (parse_url(argv[1], &host, &path) < 0) {
        return 1;
    }

    printf("Connecting to host: %s\n", host);

    struct addrinfo hints;
    struct addrinfo *res, *res0;
    int error;
    int client_connection = -1;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo(host, "http", &hints, &res0);
    if (error)
        errx(EXIT_FAILURE, "%s", gai_strerror(error));
    
    for (res = res0; res; res = res->ai_next) {
        client_connection = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (client_connection < 0)
            continue;
        
        if (connect(client_connection, res->ai_addr, res->ai_addrlen) < 0) {
            close(client_connection);
            client_connection = -1;
            continue;
        }

        break;
    }
    freeaddrinfo(res0);
    if (client_connection < 0) {
        errx(EXIT_FAILURE, "Could not connect to host");
    }

    http_get(client_connection, path, host);
    display_result(client_connection);
    printf("Shutting down\n");
    if (close(client_connection) == -1) {
        perror("Error closing connection");
        return 5;
    }


    return 0;
}


