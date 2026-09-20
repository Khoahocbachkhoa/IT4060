// Đọc và ghi các socket
#include <stdio.h>

// ta có thể coi các socket như các file
// ssize_t readn(int filedes, void *buff, size_t nbytes);
// ssize_t writen(int filedes, const void *buff, size_t nbytes);
// ssize_t readline(int filedes, void *buff, size_t maxlen);

// Đọc tối đa nbytes từ file descriptor và lưu vào buff, trả về số byte đọc được
size_t readn(int filedes, void *buff, size_t nbytes);

int main() {

}