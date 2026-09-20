#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "dns.h"

#define PORT 53
#define MAXLINE 1024

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <dns_server_ip> <domain>\n", argv[0]);
        return 1;
    }

    // Đọc dns server ip và domain từ tham số dòng lệnh
    char *ip = argv[1];
    char *dom = argv[2];

    if (strlen(dom) >= 256) {
        fprintf(stderr, "Domain name too long!\n");
        return 1;
    }

    int sockfd;
    struct sockaddr_in serv_addr;
    u_int16_t query_id = 0x1234;
    unsigned char domain[256];
    unsigned char query[512];
    unsigned char reponse[512];

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    sockfd = create_udp_socket();

    // encode domain
    if (encode_domain_name(domain, sizeof(domain), dom) <= 0) {
        fprintf(stderr, "encode domain error.\n");
        exit(EXIT_FAILURE);
    }

    // tao packet query
    int query_size;
    if ((query_size = build_dns_query(query, sizeof(query), domain, query_id)) < 0) {
        fprintf(stderr, "Build query error!\n");
        exit(EXIT_FAILURE);
    }

    // gui query cho server
    if (send_dns_query(sockfd, serv_addr, query, query_size) < 0) {
        perror("send error");
        exit(EXIT_FAILURE);
    }

    // nhan phan hoi tu dns server
    int reponse_size = receive_dns_response(sockfd, serv_addr, reponse, sizeof(reponse));
    if (reponse_size == 0) {
        fprintf(stderr, "Timeout reached!\n");
        exit(EXIT_FAILURE);
    }

    // doc phan hoi va phan tich ket qua
    if (parse_dns_reponse(reponse, reponse_size, query_id) < 0) {
        fprintf(stderr, "Parse response error!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}