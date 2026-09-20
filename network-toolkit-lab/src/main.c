#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "dns_module.h"
#include "http_module.h"
#include "ftp_module.h"
#include "smtp_module.h"
#include "pop3_module.h"

int test_dns(char *dns_server, int dns_port);
int test_http(char *dns_server, int dns_port);
int test_ftp(char *dns_server, int dns_port);
int test_smtp(char *dns_server, int dns_port);
int test_pop3(char *dns_server, int dns_port);
void test_all(char *dns_server, int dns_port);

int main(int argc, char **argv) {
    char *dns_server = NULL;
    int dns_port = 53;
    int menu_mode = 0;

    // parse tham so dong lenh
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--menu") == 0) {
            menu_mode = 1;
        } else if (strcmp(argv[i], "--full-test") == 0) {
            menu_mode = 0;
        } else if (strcmp(argv[i], "--dns-server") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error parsing --dns-server\n");
                return 1;
            }

            dns_server = argv[i];
        } else if (strcmp(argv[i], "--dns-port") == 0) {
            if (++i >= argc) {
                fprintf(stderr, "Error parsing --dns-port\n");
                return 1;
            }

            dns_port = atoi(argv[i]);
        }
    }

    if (!menu_mode) {
        test_all(dns_server, dns_port);
        return 0;
    }

    printf("===== Network Application Client Toolkit =====\n");
    printf("1. Resolve domain using DNS\n");
    printf("2. Send HTTP GET request\n");
    printf("3. Connect FTP, list files and download file\n");
    printf("4. Send email report using SMTP\n");
    printf("5. Read email using POP3\n");
    printf("6. Quit\n");
    printf("Choose an option to continue: \n");

    int choice;

    while (1) {
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                if (test_dns(dns_server, dns_port) < 0)
                    fprintf(stderr, "DNS test failed!\n");
                break;

            case 2:
                if (test_http(dns_server, dns_port) < 0)
                    fprintf(stderr, "HTTP test failed!\n");
                break;

            case 3:
                if (test_ftp(dns_server, dns_port) < 0)
                    fprintf(stderr, "FTP test failed!\n");
                break;

            case 4:
                if (test_smtp(dns_server, dns_port) < 0)
                    fprintf(stderr, "SMTP test failed!\n");
                break;

            case 5:
                if (test_pop3(dns_server, dns_port) < 0)
                    fprintf(stderr, "POP3 test failed!\n");
                break;
            case 6:
                exit(EXIT_SUCCESS);
        }
    }
    
    return 0;
}

int test_dns(char *dns_server, int dns_port) {
    char ip_str[INET_ADDRSTRLEN];

    if (dns_resolve_a(dns_server, dns_port, "web.lab.local", ip_str, INET_ADDRSTRLEN) < 0) {
        fprintf(stderr, "Cannot resolve web.lab.local\n");
        return -1;
    } else {
        printf("web.lab.local: %s\n", strcmp(ip_str, "") == 0 ? "Not found" : ip_str);
    }

    if (dns_resolve_a(dns_server, dns_port, "ftp.lab.local", ip_str, INET_ADDRSTRLEN) < 0) {
        fprintf(stderr, "Cannot resolve ftp.lab.local\n");
        return -1;
    } else {
        printf("ftp.lab.local: %s\n", strcmp(ip_str, "") == 0 ? "Not found" : ip_str);
    }

    if (dns_resolve_a(dns_server, dns_port, "mail.lab.local", ip_str, INET_ADDRSTRLEN) < 0) {
        fprintf(stderr, "Cannot resolve mail.lab.local\n");
        return -1;
    } else {
        printf("mail.lab.local: %s\n", strcmp(ip_str, "") == 0 ? "Not found" : ip_str);
    }

    return 0;
}

int test_http(char *dns_server, int dns_port) {
    return http_get_status(dns_server, dns_port, "web.lab.local", "/status");
}

int test_ftp(char *dns_server, int dns_port) {
    char ip_str[INET_ADDRSTRLEN];
    int control_sock;
    struct sockaddr_in ftp_serv_addr;

    if (dns_resolve_a(dns_server, dns_port, "ftp.lab.local", ip_str, INET_ADDRSTRLEN) < 0) {
        fprintf(stderr, "Cannot resolve ftp.lab.local\n");
        return -1;
    }

    ftp_serv_addr.sin_family = AF_INET;
    ftp_serv_addr.sin_port = htons(2121);
    inet_pton(AF_INET, ip_str, &ftp_serv_addr.sin_addr);

    if ((control_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "FTP create socket failed!\n");
        return -1;
    }

    if (connect(control_sock, (struct sockaddr*)&ftp_serv_addr, sizeof(ftp_serv_addr)) < 0) {
        fprintf(stderr, "FTP: cannot connect to ftp server!\n");
        return -1;
    }

    if (ftp_login(control_sock, "student", "student123") < 0) {
        fprintf(stderr, "Ftp test: incorrect username or password!\n");
        return -1;
    }

    if (ftp_list_files(control_sock) < 0) {
        return -1;
    }

    if (ftp_download_file(control_sock, "test.txt", "downloaded_test.txt") < 0) {
        return -1;
    }
    
    // quit
    close(control_sock);
    return 0;
}

int test_smtp(char *dns_server, int dns_port) {
    char ip_str[INET_ADDRSTRLEN];

    if (dns_resolve_a(dns_server, dns_port, "mail.lab.local", ip_str, INET_ADDRSTRLEN) < 0) {
        fprintf(stderr, "Cannot resolve mail.lab.local\n");
        return -1;
    }

    if (smtp_send_report(ip_str, 1025, "student@lab.local", "teacher@lab.local", "Network toolkit report", "This is a test email.") < 0) {
        fprintf(stderr, "Cannot connect to mail server\n");
        return -1;
    }

    return 0;
}

int test_pop3(char *dns_server, int dns_port) {
    char ip_str[INET_ADDRSTRLEN];

    if (dns_resolve_a(dns_server, dns_port, "mail.lab.local", ip_str, INET_ADDRSTRLEN) < 0) {
        fprintf(stderr, "Cannot resolve mail.lab.local\n");
        return -1;
    }

    if (pop3_find_email_by_subject(ip_str, 1110, "student", "student123") < 0) {
        fprintf(stderr, "cannot connect to mail server\n");
        return -1;
    } 

    return 0;
}

void test_all(char *dns_server, int dns_port) {
    if (test_dns(dns_server, dns_port) < 0) {
        fprintf(stderr, "DNS test failed!\n");
        return;
    }

    if (test_http(dns_server, dns_port) < 0) {
        fprintf(stderr, "HTTP test failed!\n");
        return;
    }

    if (test_ftp(dns_server, dns_port) < 0) {
        fprintf(stderr, "FTP test failed!\n");
        return;
    }

    if (test_smtp(dns_server, dns_port) < 0) {
        fprintf(stderr, "SMTP test failed!\n");
        return;
    }

    if (test_pop3(dns_server, dns_port) < 0) {
        fprintf(stderr, "POP3 test failed!\n");
        return;
    }

    printf("All test passed!\n");
}