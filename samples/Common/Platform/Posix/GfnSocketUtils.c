#include "GfnSocketUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/**
 * Creates a socket connection to a given hostname and port.
 * 
 * @param hostname The hostname to connect to.
 * @param port The port to connect to.
 * @return The socket file descriptor on success, -1 on failure.
 */
int CreateSocket(const char* hostname, int port)
{
    int sockfd;
    struct hostent* server;
    struct sockaddr_in serverAddr;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Failed to create socket\n");
        return -1;
    }
    server = gethostbyname(hostname);
    if (server == NULL)
    {
        printf("Failed to resolve hostname: %s\n", hostname);
        close(sockfd);
        return -1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((u_short)port);
    memcpy(&serverAddr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("Failed to connect to %s:%d\n", hostname, port);
        close(sockfd);
        return -1;
    }
    return sockfd;
}

/**
 * Sends an HTTP request to a given hostname and port.
 * 
 * @param sockfd The socket file descriptor.
 * @param hostname The hostname to connect to.
 * @param path The path to send the request to.
 * @param json_data The JSON data to send in the request body.
 * @return The number of bytes sent on success, -1 on failure.
 */
int SendHttpRequest(int sockfd, const char* hostname, const char* path, const char* json_data)
{
    char request[2048];
    int result;

    // Validate input parameters
    if (hostname == NULL || path == NULL || json_data == NULL) {
        printf("Invalid parameters: hostname, path, or json_data is NULL\n");
        return -1;
    }

    result = snprintf(request, sizeof(request),
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        path, hostname, strlen(json_data), json_data);
    
    // Check if the string was truncated
    if (result >= sizeof(request) || result < 0)
    {
        printf("HTTP request too long or snprintf failed\n");
        return -1;
    }
    
    return send(sockfd, request, result, 0);  // Use result instead of strlen(request)
}

/**
 * Receives an HTTP response from a given socket.
 * 
 * @param sockfd The socket file descriptor.
 * @return The HTTP response string on success, NULL on failure. Caller is responsible for freeing the returned string.
 */
char* ReceiveHttpResponse(int sockfd)
{
    char buffer[4096];
    char* response = NULL;
    char* newResponse;
    size_t totalSize = 0;
    ssize_t bytesReceived;

    // Initialize buffer to prevent processing garbage data on error
    memset(buffer, 0, sizeof(buffer));

    while ((bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0)
    {
        buffer[bytesReceived] = '\0';
        
        newResponse = realloc(response, totalSize + bytesReceived + 1);
        if (newResponse == NULL)
        {
            printf("Memory allocation failed\n");
            free(response);
            return NULL;
        }
        response = newResponse;
        
        memcpy(response + totalSize, buffer, bytesReceived + 1);
        totalSize += bytesReceived;
    }

    // Check for socket errors
    if (bytesReceived < 0)
    {
        printf("Socket receive error: %s\n", strerror(errno));
        free(response);
        return NULL;
    }

    // If no data was received at all, return NULL
    if (totalSize == 0)
    {
        printf("No data received from socket\n");
        return NULL;
    }

    return response;
}

/**
 * Cleans up a socket connection.
 * 
 * @param sockfd The socket file descriptor.
 */
void CleanupSocket(int sockfd)
{
    close(sockfd);
} 