#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ftp_module.h"
#include "socket_utils.h"

#define MAXLINE 1024

int ftp_login(int control_sock, const char *username, const char *password) {
    char buf[MAXLINE];
    int code;
    
    // Nhan greeting message
    recv_response(control_sock, buf, MAXLINE);

    snprintf(buf, MAXLINE, "USER %s\r\n", username);
    send_cmd(control_sock, buf);
    recv_response(control_sock, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 331)
        return -1;

    snprintf(buf, MAXLINE, "PASS %s\r\n", password);
    send_cmd(control_sock, buf);
    recv_response(control_sock, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 230)
        return -1;

    return 0;
}

int ftp_enter_passive_mode(int control_sock, char *data_ip, size_t data_ip_size, int *data_port) {
    char buf[MAXLINE];
    int code;

    snprintf(buf, MAXLINE, "PASV\r\n");
    send_cmd(control_sock, buf);
    recv_response(control_sock, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 227) {
        printf("PASV error\n");
        return -1;
    }

    // Parse IP and port from response
    int port, h1, h2, h3, h4, p1, p2;
    if (sscanf(buf, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &h1, &h2, &h3, &h4, &p1, &p2) != 6) {
        return -1;
    }

    *data_port = p1 * 256 + p2;
    snprintf(data_ip, data_ip_size, "%d.%d.%d.%d", h1, h2, h3, h4);

    return 0;
}

int ftp_list_files(int control_sock) {
    char buf[MAXLINE];
    int code;

    char data_ip[INET_ADDRSTRLEN];
    int data_port;

    // enter pasv mode
    if (ftp_enter_passive_mode(control_sock, data_ip, INET_ADDRSTRLEN, &data_port) < 0) {
        fprintf(stderr, "list_files: enter pasv mode error!\n");
        return -1;
    }

    // create data_sock
    int data_sock;
    if ((data_sock = connect_to_server(data_ip, data_port)) < 0) {
        fprintf(stderr, "list_files: connect to data sock error!\n");
        return -1;
    }

    // Gui lenh list
    snprintf(buf, MAXLINE, "LIST\r\n");
    send_cmd(control_sock, buf);
    recv_response(control_sock, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 150) {
        fprintf(stderr, "list_files: cannot list files!\n");
        return -1;
    }

    // Lay sanh dach file qua data socket
    int n;
    while ((n = recv(data_sock, buf, MAXLINE, 0)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
    }

    // dong data socket
    close(data_sock);
    recv_response(control_sock, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 226)
        return -1;

    return 0;
}

int ftp_download_file(int control_sock, const char *remote_file, const char *local_file) {
    char buf[MAXLINE];
    int code;

    char data_ip[INET_ADDRSTRLEN];
    int data_port;

    // enter pasv mode
    if (ftp_enter_passive_mode(control_sock, data_ip, INET_ADDRSTRLEN, &data_port) < 0) {
        fprintf(stderr, "list_files: enter pasv mode error!\n");
        return -1;
    }

    // create data_sock
    int data_sock;
    if ((data_sock = connect_to_server(data_ip, data_port)) < 0) {
        fprintf(stderr, "list_files: connect to data sock error!\n");
        return -1;
    }

    // Gui lenh RETR
    snprintf(buf, MAXLINE, "RETR %s\r\n", remote_file);
    send_cmd(control_sock, buf);
    recv_response(control_sock, buf, MAXLINE);

    sscanf(buf, "%d", &code);
    if (code != 150) {
        fprintf(stderr, "download_file: download file error!\n");
        return -1;
    }

    // Mo file de ghi
    int filefd = open(local_file, O_CREAT | O_WRONLY | O_TRUNC , S_IRUSR | S_IWUSR);
    
    int n;
    while ((n = recv(data_sock, buf, MAXLINE, 0)) > 0) {
        write(filefd, buf, n);
    }

    close(data_sock);
    close(filefd);

    recv_response(control_sock, buf, MAXLINE);
    sscanf(buf, "%d", &code);
    if (code != 226)
        return -1;

    return 1;
}