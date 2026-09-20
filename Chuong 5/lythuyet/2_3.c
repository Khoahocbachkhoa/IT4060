#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/poll.h>

int main() {
    freopen("output.txt", "w", stdout);
    freopen("input.txt", "r", stdin);
    // Theo dõi stdin và stdout với poll()
    FILE *fin = stdin, *fout = stdout;
    int infd = fileno(fin), outfd = fileno(fout);

    int i1, i2;

    struct pollfd fds[2];
    fds[1].fd = outfd;
    fds[1].events = POLLOUT;
    fds[0].fd = infd;
    fds[0].events = POLLIN;

    int ret = poll(fds, 2, -1);
    if (ret > 0) {
        if (fds[1].revents & POLLOUT) {
            printf("stdout da sang!\n");
            printf("%d\n", i1 + i2);
        }
        // Mở stdout trước stdin -> lỗi
        if (fds[0].revents & POLLIN) {
            printf("stdin da san sang!\n");
            scanf("%d %d\n", &i1, &i2);
        }
    } else if (ret == 0) {
        // timeout
    } else {
        // error
        perror("poll error:");
    }

    return 0;
}