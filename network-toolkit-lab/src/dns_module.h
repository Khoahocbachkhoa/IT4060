#ifndef DNS_H
#define DNS_H

#include <sys/types.h>

struct dns_header {
    u_int16_t id;  
    u_int16_t flags; 
    u_int16_t qdcount;
    u_int16_t ancount; 
    u_int16_t nscount;
    u_int16_t arcount;
};

// chuyen ten mien tu chuoi sang Qname
int encode_domain_name(unsigned char *buf, size_t bufsize, const char *dom);

int create_udp_socket(void);

int build_dns_query(unsigned char *query, 
                    size_t query_size, 
                    const unsigned char *qname, 
                    u_int16_t query_id);

int send_dns_query(int sockfd,
                    struct sockaddr_in serv_addr,
                    const unsigned char *query,
                    int query_len);

int receive_dns_response(int sockfd,
                        struct sockaddr_in serv_addr,
                        unsigned char *response,
                        size_t response_size);

int parse_dns_reponse(const unsigned char *reponse,
                    int response_len,
                    char *ip_buffer,
                    size_t ip_buffer_size,
                    u_int16_t expected_id);

int skip_dns_name(const unsigned char *message, int message_len, int pos);

// Truy van 1 ten mien
// Tra ve 0 neu thanh cong, nguoc lai that bai
// Neu khong ton ton tai ten mien thi ip_buf la NULL
int dns_resolve_a(const char * dns_server_ip ,
                int dns_server_port,
                const char * domain,
                char * ip_buffer,
                size_t ip_buffer_size);

#endif