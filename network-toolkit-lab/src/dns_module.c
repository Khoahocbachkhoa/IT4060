#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "dns_module.h"

// Tạo UDP socket
int create_udp_socket() {
    int sockfd;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct timeval tv;
    tv.tv_sec = 5; // 5s timeout 
    tv.tv_usec = 0;

    // Đặt timeout nhận là 5s cho socket
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    return sockfd;
}

size_t dns_name_len(const unsigned char *name) {
    size_t len = 0;

    while (name[len] != 0)
        ++len;

    return len + 1;
}

int encode_domain_name(unsigned char *buf, size_t bufsize, const char *dom) {
    if (!dom || !buf) {
        // tham so khong hop le
        return -1;
    }

    char domain[256];
    char *token;

    strcpy(domain, dom);
    token = strtok(domain, ".");

    while (token != NULL) {
        size_t len = strlen(token);

        // kiem tra kich thuoc bo dem
        if (bufsize <= len + 1) {
            return 0;
        }

        *buf++ = (unsigned char)len;
        memcpy(buf, token, len);
        buf += len;

        token = strtok(NULL, ".");
        bufsize -= len + 1;
    }

    *buf++ = 0;
    return 1; // Oke
}

// Tra ve do dai cua query
int build_dns_query(unsigned char *query, size_t query_size, const unsigned char *qname, u_int16_t query_id) {
    // tao header
    struct dns_header header;

    header.id = htons(query_id);
    header.flags = htons(0x0100); // standard query with recursion desired
    header.qdcount = htons(1);
    header.ancount = 0;
    header.arcount = 0;
    header.nscount = 0;

    // tao question section
    u_int16_t qtype = htons(1); // Truy van ban ghi A
    u_int16_t qclass = htons(1); // class = in

    size_t total = sizeof(header) + dns_name_len(qname) + sizeof(qtype) + sizeof(qclass);
    if (total > query_size)
        return -1; // Khong du bo dem de ghi

    memcpy(query, &header, sizeof(header));
    query += sizeof(header);

    memcpy(query, qname, dns_name_len(qname));
    query += dns_name_len(qname);

    memcpy(query, &qtype, sizeof(qtype));
    query += sizeof(qtype);

    memcpy(query, &qclass, sizeof(qclass));
    query += sizeof(qclass);

    return (int)total;
}

int send_dns_query(int sockfd, struct sockaddr_in serv_addr, const unsigned char *query, int query_len) {
    // gia su rang sendto da gui du so byte cua query
    int ret = sendto(sockfd, query, query_len, 0, (const struct sockaddr*)&serv_addr, sizeof(serv_addr));
    return ret;
}

int receive_dns_response(int sockfd, struct sockaddr_in serv_addr, unsigned char *response, size_t response_size) {
    socklen_t addrlen = sizeof(serv_addr);

    int n = recvfrom(sockfd, response, response_size, 0, (struct sockaddr*)&serv_addr, &addrlen);
    if (n < 0) {
        if (errno == EAGAIN)
            return 0;

        perror("recvfrom error");
        return -1;
    }

    return n;
}

int parse_dns_reponse(const unsigned char *response, int response_len, char *ip, size_t ip_size, u_int16_t expected_id) {
    struct dns_header header;
    // doc header
    memcpy(&header, response, sizeof(header));

    uint16_t flags = ntohs(header.flags);
    int rcode = flags & 0x0F;

    if (rcode == 3) {
        strcpy(ip, "");
        printf("Tên miền không tồn tại\n");
        return 0;
    }

    if (ntohs(header.id) != expected_id) {
        fprintf(stderr, "unexpected id\n");
        return -1; // khong khop id
    }

    const unsigned char *p = response + sizeof(struct dns_header);
    int an_cnt = ntohs(header.ancount);
    int qd_cnt = ntohs(header.qdcount);

    // skip dns qname, qtype and class in question section
    for (int q = 0; q < qd_cnt; q++) {
        int pos = skip_dns_name(response, response_len, p - response);

        if (pos < 0)
            return -1;

        p = response + pos + 4;
    }

    // doc answer record
    // uint16_t c_ptr; // compression pointer
    uint16_t type;
    uint16_t class;
    // uint32_t ttl;
    uint16_t rdlength;

    for (int a = 0; a < an_cnt; ++a) {
        // skip dns qname in answer section
        int pos = skip_dns_name(response, response_len, p - response);
        if (pos < 0)
            return -1;
        p = response + pos;

        // parse type, class, ttl, rdlength
        type = ntohs(*(uint16_t*)p);
        p += 2;

        class = ntohs(*(uint16_t*)p);
        p += 2;

        // ttl = ntohl(*(uint32_t*)p);
        p += 4;

        rdlength = ntohs(*(uint16_t*)p);
        p += 2;

        // Chi doc cac ban ghi A trong bai thuc hanh
        if (type == 1 && class == 1 && rdlength == 4) {
            //printf("IPv4 address: %u.%u.%u.%u\n", p[0], p[1], p[2], p[3]);
            snprintf(ip, ip_size, "%u.%u.%u.%u", p[0], p[1], p[2], p[3]);
        }

        p += rdlength;
    }

    return 0;
}

// Bo qua DNS NAME khi parse dns reponse
int skip_dns_name(const unsigned char *message, int message_len, int pos) {
    while (pos < message_len) {
        unsigned char c = message[pos];

        if (c == 0) // end of DNS name
            return pos + 1;
        
        if ((c & 0xC0) == 0xC0) // 2 bit đầu là 11 thì bỏ qua compression pointer
            return pos + 2;

        pos += c + 1;
    }

    return -1;
}

int dns_resolve_a(const char *dns_server_ip, int dns_server_port, const char *domain, char *ip_buffer, size_t ip_buffer_size) {
    int sockfd = create_udp_socket();
    
    u_int16_t query_id = 0x1234;
    unsigned char encoded_dom[256];
    unsigned char query[512];
    struct sockaddr_in serv_addr;
    unsigned char response[512];

    // Khoi tao dia chi socket cho server
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(dns_server_port);
    
    if (inet_pton(AF_INET, dns_server_ip, &serv_addr.sin_addr) < 0) {
        fprintf(stderr, "dns_resolve_a: inet pton error!\n");
        return -1;
    }

    // encode domain
    if (encode_domain_name(encoded_dom, sizeof(encoded_dom), domain) <= 0) {
        fprintf(stderr, "dns_resolve_a: encode domain error!\n");
        return -1;
    }

    // tao query packet
    int query_size;
    if ((query_size = build_dns_query(query, sizeof(query), encoded_dom, query_id)) < 0) {
        fprintf(stderr, "dns_resolve_a: build query error!\n");
        return -1;
    }

    // Gui truy van cho server
    if (send_dns_query(sockfd, serv_addr, query, query_size) < 0) {
        fprintf(stderr, "dns_resolve_a: send dns query error\n");
        return -1;
    }

    // Nhan phan hoi tu server
    int response_size = receive_dns_response(sockfd, serv_addr, response, sizeof(response));
    if (response_size == 0) {
        fprintf(stderr, "dns_resolve_a: timeout reached!\n");
        return -1;
    }

    // Nhan phan hoi tu server
    if (parse_dns_reponse(response, response_size, ip_buffer, ip_buffer_size, query_id) < 0) {
        fprintf(stderr, "Parse response error\n");
        return -1;
    }

    return 0;
}