#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

// Gui command qua sockfd
void send_cmd(int sockfd, const char *cmd);

// Doc response tu sockfd
int recv_response(int sockfd, char *buf, int sz);

// Ket noi toi server
int connect_to_server(const char *ip, int port);

// Gui het data qua socket
int send_all(int fd, const char *buf);

#endif