#include "GfnSocketUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

/**
 * Creates a socket connection to a given hostname and port.
 * 
 * @param hostname The hostname to connect to.
 * @param port The port to connect to.
 * @return The socket handle on success, INVALID_SOCKET on failure.
 */
SOCKET CreateSocket(const char* hostname, int port)
{
    // Initialize Winsock on Windows
    WSADATA wsaData;
    SOCKET sockfd;
    struct hostent* server;
    struct sockaddr_in serverAddr;
    
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed\n");
        return INVALID_SOCKET;
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET)
    {
        printf("Failed to create socket\n");
        WSACleanup();
        return INVALID_SOCKET;
    }
    server = gethostbyname(hostname);
    if (server == NULL)
    {
        printf("Failed to resolve hostname: %s\n", hostname);
        closesocket(sockfd);
        WSACleanup();
        return INVALID_SOCKET;
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons((u_short)port);
    memcpy(&serverAddr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    
    if (connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Failed to connect to %s:%d\n", hostname, port);
        closesocket(sockfd);
        WSACleanup();
        return INVALID_SOCKET;
    }
    return sockfd;
}

/**
 * Sends an HTTP request to a given hostname and port.
 * 
 * @param sockfd The socket handle.
 * @param hostname The hostname to connect to.
 * @param path The path to send the request to.
 * @param json_data The JSON data to send in the request body.
 * @return The number of bytes sent on success, SOCKET_ERROR on failure.
 */
int SendHttpRequest(SOCKET sockfd, const char* hostname, const char* path, const char* json_data)
{
    // Validate input parameters
    char request[2048];
    int result;
    
    if (hostname == NULL || path == NULL || json_data == NULL) {
        printf("Invalid parameters: hostname, path, or json_data is NULL\n");
        return SOCKET_ERROR;
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
        return SOCKET_ERROR;
    }
    
    return send(sockfd, request, result, 0);  // Use result instead of strlen(request)
}

/**
 * Receives an HTTP response from a given socket.
 * 
 * @param sockfd The socket handle.
 * @return The HTTP response string on success, NULL on failure. Caller is responsible for freeing the returned string.
 */
char* ReceiveHttpResponse(SOCKET sockfd)
{
    char buffer[4096];
    char* response = NULL;
    char* newResponse;
    size_t totalSize = 0;
    SSIZE_T bytesReceived;
    int error;

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
        error = WSAGetLastError();
        printf("Socket receive error: %d\n", error);
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
 * @param sockfd The socket handle.
 */
void CleanupSocket(SOCKET sockfd)
{
    closesocket(sockfd);
    WSACleanup();
} 