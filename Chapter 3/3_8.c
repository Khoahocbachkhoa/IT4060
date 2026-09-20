#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// Các hàm tiện ích khi làm việc với sock_addr

// Trả về địa chỉ của socket dạng ip:port
char *sock_ntop(const struct sockaddr *sa, socklen_t addrlen) {
    char portstr[8];
    static char ans[128]; /* kich thuoc lon nhat la 128 cua unix domain */

    sa_family_t type = sa->sa_family;
    if (type == AF_INET) {
        // sockaddr là cấu trúc tổng quát, cần chuyển sang sockaddr_in
        struct sockaddr_in *sin = (struct sockaddr_in*)sa;
        // Chuyển đổi địa chỉ IP của socket sang dạng thập phân có chấm
        if (inet_ntop(AF_INET, &sin->sin_addr, ans, sizeof(ans)/sizeof(char)) == NULL)
            return NULL;
        // In gia tri so cong vao portstr
        if (sin->sin_port != 0) {
            snprintf(portstr, sizeof(portstr), ":%d", sin->sin_port);
            strcat(ans, portstr);
        }
    } else {
        printf("Unsopported!");
    }

    return ans;
}

// Một số hàm tiện ích khác tham khảo trong sách, logic hoàn toàn tương tự

int main() {
    struct sockaddr_in sin;
    struct in_addr iaddr;
    in_port_t port = 8080;

    if (inet_pton(AF_INET, "192.168.1.1", (void*)&iaddr) <= 0) {
        printf("An error occured!\n");
        exit(0);
    }

    sin.sin_family = AF_INET;
    sin.sin_addr = iaddr;
    sin.sin_port = port;

    char *saddr = sock_ntop((struct sockaddr*)&sin, sizeof(sin));
    if (saddr != NULL)
        printf("%s\n", saddr);

    return 0;
}