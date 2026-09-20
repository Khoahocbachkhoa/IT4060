#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>

#include "http_module.h"
#include "dns_module.h"
#include "socket_utils.h"

int http_get_status(const char *server_ip, int server_port, const char *host_header, const char *path) {
    // Goi DNS de phan giai ten mien web server
    char ip_str[INET_ADDRSTRLEN];
    if (dns_resolve_a(server_ip, server_port, "web.lab.local", ip_str, sizeof(ip_str)) < 0) {
        fprintf(stderr, "http_get_status: cannot resolve ip address\n");
        return -1;
    }

    if (strcmp(ip_str, "") == 0) {
        fprintf(stderr, "http_get_status: domain web.lab.local not found!\n");
        return -1;
    }

    int sockfd;
    char response[8192];

    sockfd = connect_to_server(ip_str, 8080);
    if (sockfd < 0) {
        fprintf(stderr, "http_get_status: socket creation failed!\n");
        return -1;
    }

    // Gui HTTP request
    char buf[1024];
    snprintf(buf, sizeof(buf), "GET %s HTTP/1.1\r\n", path);
    send(sockfd, buf, strlen(buf), 0);
    snprintf(buf, sizeof(buf), "Host: %s\r\n", host_header);
    send(sockfd, buf, strlen(buf), 0);
    snprintf(buf, sizeof(buf), "Connection: close\r\n\r\n");
    send(sockfd, buf, strlen(buf), 0);

    // Nhan HTTP response
    recv(sockfd, response, sizeof(response), 0);
    
    char version[16];
    int code;
    char reason[64];

    sscanf(response, "%15s %d %[^\r\n]", version, &code, reason);
    printf("%d %s\n", code, reason);

    char *body = strstr(response, "\r\n\r\n");
    body += 4;
    printf("%s\n", body);
    return 0;
}
