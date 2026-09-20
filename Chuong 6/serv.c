#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#define PORT 8080
#define MAXLINE 1024
#define KEY "a&8da#$@fsdhjk"
#define MAXCLIENT 10
#define TIMEOUT 10

void xor_cipher(char *data, int len, char *key);
char *sock_ntop(const struct sockaddr *sa, socklen_t addrlen);
int sock_cmp(struct sockaddr_in *a, struct sockaddr_in *b);

/*
    The client sends a UDP message to the server to initiate the connection.
    The server sends back the message to the client.
    The client response with a message.
    The server sends the message "Goodbye, see you again!” to client.
*/

struct Client {
    struct sockaddr_in cliaddr;
    time_t last_sent; // when server sent message to client
};

int main(int argc, char const *argv[]) {
    int i, maxi = -1;
    int sockfd;
    char buf[MAXLINE];
    char ack[MAXLINE] = "Goodbye, see you again!";
    char messages[4][MAXLINE] = {
        "How are you today?", 
        "The weather is very nice, isnt it?", 
        "What do you do in your free time?",
        "Do you like tomato?"
    };
    struct sockaddr_in servadddr, cliaddr;
    struct Client* clients[MAXCLIENT];

    // init server
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed!");
        exit(EXIT_FAILURE);
    }

    memset(&servadddr, 0, sizeof(servadddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servadddr.sin_family = AF_INET;
    servadddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servadddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&servadddr, sizeof(servadddr)) < 0) {
        perror("bind error!");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server is running in port: %d\n", PORT);

    // init the clients array
    for (i = 0; i < MAXCLIENT; ++i)
        clients[i] = NULL;

    ssize_t n;
    fd_set rset;
    struct timeval tv;
    for (;;) {
        FD_ZERO(&rset);
        FD_SET(sockfd, &rset);

        tv.tv_sec = 1; // each 1s, check for connection or timeout
        tv.tv_usec = 0;

        int nready = select(sockfd+1, &rset, NULL, NULL, &tv);

        // if sockfd is ready
        if (nready > 0 && FD_ISSET(sockfd, &rset)) {
            socklen_t len = sizeof(cliaddr);
            // recv message from a client
            if ((n = recvfrom(sockfd, buf, MAXLINE, 0, (struct sockaddr*)&cliaddr, &len)) == -1) {
                perror("read error");
                exit(EXIT_FAILURE);
            }
            buf[n] = '\0';
            xor_cipher(buf, n, KEY);
            printf("%s : %s\n", sock_ntop((struct sockaddr*)&cliaddr, sizeof(cliaddr)), buf);

            int flag = 1;
            // if this client already connect to server
            for (i = 0; i <= maxi; ++i) {
                if (clients[i] != NULL && sock_cmp(&cliaddr, &clients[i]->cliaddr)) {
                    // send ack to client
                    strcpy(buf, ack);
                    int len_msg = strlen(buf);
                    xor_cipher(buf, len_msg, KEY);
                    sendto(sockfd, buf, len_msg, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                    printf("To %s : %s\n", sock_ntop((struct sockaddr*)&cliaddr, sizeof(cliaddr)), ack);
                    // free clients array
                    free(clients[i]);
                    clients[i] = NULL;
                    flag = 0;
                    break;
                }
            }
            // else this is a new client
            if (flag == 1) {
                for (i = 0; i < MAXCLIENT; ++i) {
                    if (clients[i] == NULL) {
                        clients[i] = malloc(sizeof(struct Client));
                        clients[i]->cliaddr = cliaddr;
                        break;
                    }
                }
                if (i == MAXCLIENT) {
                    printf("Maximum clients has been reached! Refused connection from: %s\n", 
                        sock_ntop((struct sockaddr*)&cliaddr, sizeof(cliaddr)));
                    continue;
                }
                if (i > maxi) maxi = i;
                // send random message to client
                strcpy(buf, messages[i%4]);
                printf("To %s : %s\n", sock_ntop((struct sockaddr*)&cliaddr, sizeof(cliaddr)), buf);
                int len_msg = strlen(buf);
                xor_cipher(buf, len_msg, KEY);
                sendto(sockfd, buf, len_msg, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                // set timeout for this client
                clients[i]->last_sent = time(NULL);
            }
        } else {
            // check if any clients reached timeout
            time_t now = time(NULL);

            for (i = 0; i <= maxi; ++i) {
                if (clients[i] != NULL && now - clients[i]->last_sent >= TIMEOUT) {
                    printf("Timeout for client %s\n", 
                        sock_ntop((struct sockaddr*)&clients[i]->cliaddr, sizeof(clients[i]->cliaddr)));
                    free(clients[i]);
                    clients[i] = NULL;
                }
            }
        }
    }

    return 0;
}

char *sock_ntop(const struct sockaddr *sa, socklen_t addrlen) {
    char portstr[8];
    static char ans[128];

    sa_family_t type = sa->sa_family;
    if (type == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in*)sa;
        if (inet_ntop(AF_INET, &sin->sin_addr, ans, sizeof(ans)/sizeof(char)) == NULL)
            return NULL;
        if (sin->sin_port != 0) {
            snprintf(portstr, sizeof(portstr), ":%d", ntohs(sin->sin_port));
            strcat(ans, portstr);
        }
    }

    return ans;
}
void xor_cipher(char *data, int len, char *key) {
    int key_len = strlen(key);
    for (int i = 0; i < len; ++i) {
        data[i] ^= key[i % key_len];
    }
}
int sock_cmp(struct sockaddr_in *a, struct sockaddr_in *b) {
    return (a->sin_family == b->sin_family) &&
           (a->sin_port == b->sin_port) &&
           (a->sin_addr.s_addr == b->sin_addr.s_addr);
}