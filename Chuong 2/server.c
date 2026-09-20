#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     //* cung cấp các hàm gọi system call
#include <arpa/inet.h>  //* cung cấp các thao tác với địa chỉ IP

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address; //* Biến cấu trúc địa chỉ IPv4
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    const char *hello = "Hello from server";

    /*
        * int socket(int domain, int type, int protocol)
        * Mô tả: tạo một socket
        * domain: xác định họ giao thức (AP_INET <-> IPv4)
        * type: kiểu giao tiếp (SOCK_STREAM -> TCP socket)
        * protocol: kiểu giao thức (thường đặt là 0)
        * trả về: > 0 nếu thành công và lỗi nếu ngược lại
    */
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind to address and port
    //* Khi tạo xong socket, cần gắn (bind) với 1 ip và port cụ thể
    address.sin_family = AF_INET; //* Ipv4
    address.sin_addr.s_addr = htonl(INADDR_ANY); //* Gán vào tất cả các card mạng đang có!
    address.sin_port = htons(PORT); //* Đặt số cổng là 80

    // INADDR_ANY: địa chỉ ip cho kernel tự chọn
    // kernel sẽ chỉ chọn địa chỉ ip cho tới khi nhận được 1 kết nối từ client

    /*
        * int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        * Mô tả: bind socket vào address
        * server_fd : trả về của socket object trước đó
        * address: đối tượng sockaddr trước đó đã gán
        * Trả về 0 nếu bind thành công và -1 nếu lỗi
    */
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    //* Bắt đầu lắng nghe kết nối
    /*
        * int listen(int sockfd, int backlog);
        * sockfd : mô tả của socket đã tạo trước đó
        * backlog : số kết nối tối đa trong hàng đợi
    */
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept a connection from the client
    //* Lấy một kết nối đã hoàn tất, tạo socket mới dành riêng cho client đó và trả về thông tin kết nối (new_socket)
    // address chứa thông tin client ip và port
    // * tham số thứ 2 và 3 có thể đặt là null nếu ko cần biết danh tính của client
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    //! Đọc và ghi thực hiện trên new_socket

    // Read data sent by the client
    read(new_socket, buffer, BUFFER_SIZE);
    printf("Message from client: %s\n", buffer);

    // Send a response to the client
    send(new_socket, hello, strlen(hello), 0);
    printf("Hello message sent to client\n");

    // Close the socket
    close(new_socket);
    close(server_fd);

    return 0;
}