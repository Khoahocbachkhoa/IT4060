#include <netinet/in.h>
#include <sys/types.h>

int main() {
    struct in_addr iadd; // cấu trúc biểu diễn một địa chỉ ipv4
    iadd.s_addr; // địa chỉ ipv4

    int16_t i;
    struct sockaddr_in port_add; // cấu trúc biểu diễn địa chỉ của một công ứng dụng

    struct sockaddr_in6 port16_add; // cấu trúc biểu diễn địa chỉ của một cổng ứng dụng dùng Ipv6
    // chi tiết về các thành phần xem giáo trình
}