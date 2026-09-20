#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/poll.h>

int main() {
    struct pollfd fds[1];

    //int fd = socket(AF_INET, SOCK_STREAM, 0);
    int fd = 10;

    fds[0].fd = fd; // đây là một FD không hợp lệ!
    fds[0].events = POLLIN;
    int ret = poll(fds, 1, 1000);

    if (ret > 0) {
        // Kiểm tra lỗi bằng POLLNVAL
        //* fd không hợp lệ cũng là 1 sự kiện!
        if (fds[0].revents & POLLNVAL) {
            printf("Fd khong hop le!\n");
        }
    } else if (ret == 0) {
        // timeout
        printf("Time out!\n");
    } else {
        perror("Poll error: ");
    }

    return 0;
}