#ifndef FTP_H
#define FTP_H

#include <unistd.h>

int recv_response(int sockfd, char *buf, int sz);

int ftp_login(int control_sock, const char *username, const char *password);

int ftp_enter_passive_mode(int control_sock, char *data_ip, size_t data_ip_size, int *data_port);

int ftp_list_files(int control_sock);

int ftp_download_file(int control_sock, const char *remote_file, const char *local_file);

#endif