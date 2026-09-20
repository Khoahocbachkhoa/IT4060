#include <stdio.h>
#include <string.h>
#include <sys/types.h>

// union là cấu trúc chia sẽ chung bộ nhớ nên kiểm tra rất mạnh

union un {
    int16_t i;
    char c[sizeof(int16_t)];
};


int main_() {
    // Xác định xem hệ thống là big-endian hay little-endian
    union un u;
    u.i = 0x0102;

    // printf("%d\n", u.c[0]);
    // printf("%d\n", u.c[1]);

    if (u.c[0] == 0x01 && u.c[1] == 0x02) {
        // 0x01 là byte cao, c[0] có địa chỉ ở dưới c[1]
        // byte cao ở dưới -> big endian
        printf("Big endian!");
    } else {
        printf("Little endian");
    }

    return 0;
}

#include <netinet/in.h>

int main() {
    //* uint16_t htons(uint16_t host16bitvalue);
    //* uint32_t htonl(uint32_t host32bitvalue);
    /* Trả về: giá trị theo network byte order */

    //* uint16_t ntohs(uint16_t net16bitvalue);
    //* uint32_t ntohl(uint32_t net32bitvalue);
    /* Trả về: giá trị theo host byte order */
    //? u : unsigned
    //? h: host, to, n : network, s: short, l: long
    //? hiểu là chuyển qua lại giữa các hệ thống giúp đồng bộ
}