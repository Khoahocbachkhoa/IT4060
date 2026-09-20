// File .h dùng để khai báo, chưa định nghĩa! (đây là tiêu chuẩn chung)

#ifndef SOCK_NTOP_H
#define SOCK_NTOP_H

#include <sys/socket.h>

char *sock_ntop(const struct sockaddr *sa, socklen_t salen);

#endif
