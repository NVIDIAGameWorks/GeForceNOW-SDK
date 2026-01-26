#ifndef __GFN_SOCKET_UTILS_H__
#define __GFN_SOCKET_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
    // Include winsock2.h before windows.h to prevent conflicts
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
#endif

    // Description
    // Creates a socket connection to a given hostname and port.
    //
    // Parameters
    // hostname The hostname to connect to.
    // port The port to connect to.
    //
    // Return Values
    // The socket handle on success, INVALID_SOCKET on Windows or -1 on POSIX systems on failure.
#ifdef _WIN32
    SOCKET CreateSocket(const char* hostname, int port);
#else
    int CreateSocket(const char* hostname, int port);
#endif

    // Description
    // Sends an HTTP request to a given hostname and port.
    //
    // Parameters
    // sockfd The socket handle.
    // hostname The hostname to connect to.
#ifdef _WIN32
    int SendHttpRequest(SOCKET sockfd, const char* hostname, const char* path, const char* json_data);
#else
    int SendHttpRequest(int sockfd, const char* hostname, const char* path, const char* json_data);
#endif

    // Description
    // Receives an HTTP response from a given socket.
    //
    // Parameters
    // sockfd The socket handle.
    //
    // Return Values
    // The HTTP response string on success, NULL on failure. Caller is responsible for freeing the returned string.
#ifdef _WIN32
    char* ReceiveHttpResponse(SOCKET sockfd);
#else
    char* ReceiveHttpResponse(int sockfd);
#endif

    // Description
    // Cleans up a socket connection.
    //
    // Parameters
    // sockfd The socket handle.
#ifdef _WIN32
    void CleanupSocket(SOCKET sockfd);
#else
    void CleanupSocket(int sockfd);
#endif

#ifdef __cplusplus
}
#endif

#endif // __GFN_SOCKET_UTILS_H__ 