#include <stdio.h>
#include <stdlib.h>

#include "smtp_module.h"
#include "socket_utils.h"

int smtp_send_report(char *server_ip, int server_port, const char *sender, const char *recipient, const char *subject, const char *body) {
    int sockfd = connect_to_server(server_ip, server_port);
    if (sockfd < 0) {
        fprintf(stderr, "cannot connect to server\n");
        return -1;
    }

    char req[256];
    char buf[1024];

    // Send hello message
    snprintf(req, sizeof(req), "HELO localhost\r\n");
    send_cmd(sockfd, req);

    snprintf(req, sizeof(req), "MAIL FROM:<%s>\r\n", sender);
    send_cmd(sockfd, req);

    snprintf(req, sizeof(req), "RCPT TO:<%s>\r\n", recipient);
    send_cmd(sockfd, req);

    snprintf(req, sizeof(req), "DATA\r\n");
    send_cmd(sockfd, req);

    snprintf(buf, sizeof(buf), "Subject: %s\r\n\r\n", subject);
    send_all(sockfd, buf);

    // Gui het body
    send_all(sockfd, body);
    // Gui 1 dong co dau cham de ket thuc
    send_all(sockfd, "\r\n.\r\n");

    // Quit
    snprintf(req, sizeof(req), "QUIT\r\n");
    send_cmd(sockfd, req);

    return 0;
}