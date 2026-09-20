#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>

int main(int argc, char const *argv[])
{
    int fd1 = 10, fd2 = 11, fd3 = 20; // gia su
    fd_set read_fds;

    int max_fd = 20;

    FD_ZERO(&read_fds);
    // Gia su set fd1
    FD_SET(fd1, &read_fds);

    int ret = select(max_fd+1, &read_fds, 0, 0, 0);

    if (ret > 0) {
        for (int i = 0; i <= max_fd; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                // Xu ly socket co file descriptor la i
                printf("Descriptor i san sang de doc!\n");
            }
        }
    } else if (ret == 0) {
        // timeout
    } else {
        // error
    }

    //? Cần hiểu cơ chế tham số dùng và trả về của select
    //* Đơn giản là nếu có fd nào sẵn sàng thì set bit tương ứng fd đó, các bit còn lại sẽ được clear là 0
    //* trước khi dùng select, chú ý cần FD_ZERO để khởi tạo . 1_11
    
    return 0;
}
