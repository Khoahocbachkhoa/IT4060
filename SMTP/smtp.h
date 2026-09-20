#ifndef SMTP_H
#define SMTP_H

#include <stddef.h>

int connect_to_server(const char *ip, int port);

int send_all(int fd, const char *buf);

int recv_response(int fd, char *buf, size_t buf_size);

int check_response_code(const char *res, const char *expected_code);

int send_cmd(int fd, const char *cmd, const char *expected_code);

int send_header(int fd, const char *from, const char *to, const char *sub);

int read_email_body_and_send(int fd);

#endif // !SMTP_H
