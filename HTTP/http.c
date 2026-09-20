#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "http.h"

#define MAXLINE 1024
#define HEADER_SIZE 4096
#define BODY_SIZE 8192

int readline(int connfd, char *buf, int maxlen) {
    int i = 0;
    char c;

    while (i < maxlen - 1) {
        int n = recv(connfd, &c, 1, 0);
        if (n <= 0) return -1; // error or disconnected
        buf[i++] = c;
        if (i >= 2 && buf[i-1] == '\n' && buf[i-2] == '\r')
            break; // end of line
    }

    buf[i] = '\0';
    return i;
}

void uri_to_path(const char *uri, char *path) {
    if (uri[strlen(uri)-1] == '/')
        snprintf(path, MAXLINE, ".%sindex.html", uri);
    else
        snprintf(path, MAXLINE, ".%s", uri);
}

const char *get_content_type(const char *path) {
    char *ext = strrchr(path, '.');

    if (!ext) return "application/octet-stream";
    
    if (strcmp(ext, ".html") == 0) return "text/html; charset=UTF-8";
    if (strcmp(ext, ".css") == 0)  return "text/css; charset=UTF-8";
    if (strcmp(ext, ".txt") == 0)  return "text/plain; charset=UTF-8";

    return "application/octet-stream";
}

// flag : true -> send body
// flag : false -> dont send body
void handle_path(int connfd, char *path, int flag) {
    struct stat st;
    char version[32] = "HTTP/1.1";
    char body[BODY_SIZE];
    char header[HEADER_SIZE];

    if (stat(path, &st) < 0 || !S_ISREG(st.st_mode)) {
        // file not found or not a normal file
        snprintf(body, sizeof(body), "<h1>404 Not Found</h1>");
        snprintf(header, sizeof(header),
            "%s 404 Not Found\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n"
            "\r\n",
            version, strlen(body));

        send(connfd, header, strlen(header), 0);
        if (flag) send(connfd, body, strlen(body), 0);
        return;
    }

    // If file exist
    snprintf(header, sizeof(header),
        "%s 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n", version, get_content_type(path), st.st_size);
    
    send(connfd, header, strlen(header), 0);
    // read file and send body
    if (flag) {
        int filefd = open(path, O_RDONLY);

        int n;
        while ((n = read(filefd, body, sizeof(body))) > 0) {
            send(connfd, body, n, 0);
        }

        close(filefd);
    }
}

// flag = 0: post
// flag = 1: put
void handle_post_or_put(int connfd, int flag) {
    char version[32] = "HTTP/1.1";
    char buf[MAXLINE];
    char body[BODY_SIZE];
    char header[HEADER_SIZE];
    int content_len;

    for (;;) {
        readline(connfd, buf, MAXLINE); // read a header line
        if (strcmp(buf, "\r\n") == 0) // empty line
            break;
        if (strncmp(buf, "Content-Length:", strlen("Content-Length:")) == 0) {
            // parse content length
            sscanf(buf, "Content-Length: %d\r\n", &content_len);
        }
    }

    // Write body to post_data.txt or put_data.txt
    char filename[MAXLINE];
    if (flag == 0)
        strcpy(filename, "post_data.txt");
    else
        strcpy(filename, "put_data.txt");
    
    int filefd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int remain = content_len, n;
    while (remain > 0) {
        n = read(connfd, body, sizeof(body));
        if (n <= 0) break;
        remain -= n;
        write(filefd, body, n);
    }

    // Send response to client
    char response_body[MAXLINE];
    if (flag == 0)
        strcpy(response_body, "<h1>POST SUCCESS</h1>");
    else
        strcpy(response_body, "<h1>PUT SUCCESS</h1>");

    snprintf(header, sizeof(header),
        "%s 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n", version, strlen(response_body)
        );
    send(connfd, header, strlen(header), 0);
    send(connfd, response_body, strlen(response_body), 0);

    close(filefd);
}

void handle_delete(int connfd, char *path) {
    struct stat st;
    char version[32] = "HTTP/1.1";
    char header[HEADER_SIZE];
    int status_code = 200;

    // if file not found
    if (stat(path, &st) < 0) {
        status_code = 404;
    } else {
        // pretend to delete the file
    }
    char response_body[MAXLINE];
    if (status_code == 200)
        strcpy(response_body, "<h1>The deletion successfully!</h1>");
    else
        strcpy(response_body, "<h1>Uri not found!</h1>");

    snprintf(header, sizeof(header),
        "%s %d %s\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n", version, status_code, (status_code == 200) ? "OK" : "Not Found", strlen(response_body));
    
    send(connfd, header, strlen(header), 0);
    send(connfd, response_body, strlen(response_body), 0);
}

void handle_request(int connfd) {
    char method[16], uri[256], version[32];
    char line[MAXLINE], path[MAXLINE];
    // read request line
    readline(connfd, line, MAXLINE);
    // parse method, uri and version
    sscanf(line, "%s %s %s\r\n", method, uri, version);

    if (strcmp(method, "GET") == 0) {
        // convert uri to path
        uri_to_path(uri, path);
        // check the path and send response
        handle_path(connfd, path, 1);
    } else if (strcmp(method, "POST") == 0) {
        handle_post_or_put(connfd, 0);
    } else if (strcmp(method, "HEAD") == 0) {
        // convert uri to path
        uri_to_path(uri, path);
        // check the path and send response
        handle_path(connfd, path, 0);
    } else if (strcmp(method, "PUT") == 0) {
        handle_post_or_put(connfd, 1);
    } else if (strcmp(method, "DELETE") == 0) {
        // conver uri to path
        uri_to_path(uri, path);
        // check if exist and delete
        handle_delete(connfd, path);
    } else {
        // method not supported!
    }
}
