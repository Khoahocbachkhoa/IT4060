#ifndef HTTP_H
#define HTTP_H

int http_get_status(const char *server_ip, int server_port, const char *host_header, const char *path);

#endif