#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>

int main(int argc, char const *argv[])
{
    fd_set read_fds;
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // Chung ta co the cau hinh va quan ly them mot socket nua
    int sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
    
    FD_ZERO(&read_fds);
    // them sockfd vao tap can theo doi
    //? FD_SET(sockfd, &read_fds);
    FD_SET(sockfd2, &read_fds);

    // Co the cau hinh timeout cho ham select
    struct timeval timeout;
    timeout.tv_sec = 10; // timeout 10s
    timeout.tv_usec = 0;

    int max_sockfd = (sockfd > sockfd2) ? sockfd : sockfd2;

    // * chú ý tham số đầu tiên
    int ret = select(max_sockfd+1, &read_fds, NULL, NULL, &timeout);

    if (ret > 0) {
        if (FD_ISSET(sockfd, &read_fds)) {
            printf("Socket san sang de doc!\n");
        }
        //* Thực hiện đọc trên socket với hàm recv

        if (FD_ISSET(sockfd2, &read_fds)) {
            printf("Socket 2 san sang de doc!\n");
        }
    } else if (ret == 0) {
        printf("Co timeout xay ra!\n");
    } else {
        printf("Co loi xay ra!\n");
    }

    return 0;
}
