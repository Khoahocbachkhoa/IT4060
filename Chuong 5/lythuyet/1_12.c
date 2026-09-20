#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>

int main() {
    //* phai la socket that, ko phai so nguyen!
    int listener = socket(AF_INET, SOCK_STREAM, 0), client = socket(AF_INET, SOCK_STREAM, 0);
    fd_set rset;
    int max_fd = (listener > client) ? listener : client;
    //* dung file_no de chuyen stdin, stdout ve socket!
    FILE *fp = stdin;
    int stdin_fd = fileno(fp);
    max_fd = (max_fd > stdin_fd) ? max_fd : stdin_fd;

    FD_ZERO(&rset);
    FD_SET(listener, &rset);
    FD_SET(client, &rset);
    FD_SET(stdin_fd, &rset);
    
    /* int ival;
    scanf("%d", &ival);
    printf("%d\n", ival); */

    while (1)
    {
        int ret = select(max_fd+1, &rset, NULL, NULL, NULL);
        if (ret > 0) {
            if (FD_ISSET(listener, &rset)) {
                printf("Listener san sang lang nghe!\n");
                //... thuc hien IO cho listenr
            }
            if (FD_ISSET(client, &rset)) {
                printf("Client san sang lang nghe!\n");
                //... thuc hien IO cho client
            }
            // kiem tra stdin
            if (FD_ISSET(stdin_fd, &rset)) {
                printf("Stdin san sang de doc!\n");
                //... doc tu dau vao chuan
                int i;
                scanf("%d", &i);
                printf("%d\n", i);
            }
        } else {
            // Error hoac timeout
        }
        FD_ZERO(&rset);
    }

    return 0;
}