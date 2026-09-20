// File .c dùng để triển khai
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "sock_ntop.h"

#define MAX_ADDR_STRLEN 128

// Custom sock_ntop function to convert a socket address into a string (IP and port)
char *sock_ntop(const struct sockaddr *sa, socklen_t salen) {
    static char str[MAX_ADDR_STRLEN];   // Buffer to hold the string representation
    char portstr[8];                    // Buffer to hold the port as a string

    // Check if the address is IPv4
    if (sa->sa_family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *) sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL) {
            return NULL;   // Return NULL on failure
        }
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port)); // Convert port to string
        strcat(str, portstr);   // Append the port to the IP string
        return str;
    }
    // Check if the address is IPv6
    else if (sa->sa_family == AF_INET6) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
        if (inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == NULL) {
            return NULL;   // Return NULL on failure
        }
        snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin6->sin6_port)); // Convert port to string
        strcat(str, portstr);   // Append the port to the IP string
        return str;
    } else {
        return NULL;  // Unsupported address family
    }
}
