// select function
#include <stdio.h>
#include <sys/select.h> // khai bao select
#include <sys/time.h> // khai bao cac truc timeval

//* Cho phép tiến trình yêu cầu kernel chờ môt trong nhiều sự kiện xảy ra
//* Kernel chỉ đánh thức tiến trình khi có một hoặc nhiều sự kiện xuất hiện, hoặc qua một khoảng thời gian

int main(int argc, char const *argv[])
{
    // select(): bật bit tương ứng khi descriptor đó sẵn sàng để IO, xóa tất cả các des khác về 0 -> cần theo dõi
    // trả về 0 nếu timeout, trả về -1 nếu có lỗi gì đó
    return 0;
}
