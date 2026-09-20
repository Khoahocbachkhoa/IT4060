#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/poll.h>

//* int poll(struct pollfd *fds, nfds_t nfds, int timeout);
//* struct pollfd {
//    int   fd;       // descriptor
//    short events;   // sự kiện muốn theo dõi (input)
//    short revents;  // sự kiện xảy ra (output)
//};
// nfds: so fds
// timeout: thoi gian doi
// tra ve so fd san sang !
// kiem tra : fds[0].revents & POLLIN : san sang de doc
// .revents && POLLOUT: san sang de ghi

int main() {
    struct pollfd fds[1];
    freopen("input.txt", "r", stdin);
    fds[0].fd = 0; // theo doi stdin
    fds[0].events = POLLIN; // theo doi su kien doc

    int ret = poll(fds, 1, 3000);
    if (ret > 0) {
        if (fds[0].revents & POLLIN) {
            printf("Co du lieu tu stdin!\n");
            char message[1028];
            scanf("%s", message);
            int ival;
            scanf("%d", &ival);
            printf("Du lieu doc duoc: %s %d\n", message, ival);
        }
    } else if (ret == 0) {
        printf("Time out! Ko co du lieu tu stdin!\n");
    } else {
        printf("Co loi xay ra!\n");
    }

    return 0;
}