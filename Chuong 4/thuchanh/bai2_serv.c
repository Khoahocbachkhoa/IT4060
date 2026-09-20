#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXLINE 512
#define PORT 8111

struct Question {
    char question[MAXLINE];
    char options[4][MAXLINE];
    char correct[MAXLINE];
};

// tập các câu hỏi
struct Question questions[10] = {
    {"Ngon ngu nao dung de viet he dieu hanh linux", {"C", "C++", "JAVA", "PY"}, "C"},
    {"Heap trong C la gi", {"Vung nho dong", "Vung nho tinh", "Vung nho toan cuc", "Vung nho cuc bo"}, "Vung nho dong"},
    {"Con tro trong C dung de lam gi", {"Luu dia chi", "Luu gia tri", "Luu kieu du lieu", "Luu mang"}, "Luu dia chi"},
    {"Malloc tra ve gia tri gi khi cap phat that bai", {"NULL", "0", "-1", "Undefined"}, "NULL"},
    {"Struct trong C dung de", {"Gom nhieu bien khac kieu", "Gom nhieu bien cung kieu", "Khai bao ham", "Tao mang"}, "Gom nhieu bien khac kieu"},
    {"sizeof(int) thong thuong bang bao nhieu byte", {"2", "4", "8", "Tuy he thong"}, "4"},
    {"Mang trong C la", {"Tap hop cac phan tu cung kieu", "Tap hop cac bien", "Con tro", "Struct"}, "Tap hop cac phan tu cung kieu"},
    {"Ternary operator trong C la", {"a ? b : c", "a && b", "a || b", "!a"}, "a ? b : c"},
    {"EOF trong C la gi", {"End of File", "End of Function", "Error on File", "End of Frame"}, "End of File"},
    {"strlen() tra ve", {"Do dai chuoi khong tinh null terminator", "Do dai chuoi tinh null terminator", "Ky tu dau chuoi", "Con tro chuoi"}, "Do dai chuoi khong tinh null terminator"}
};

void handle_cli(int sockfd);

int main() {
    pid_t pid;
    int listenfd, connfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 3);

    while (1) {
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);

        pid = fork();
        if (pid == 0) {
            close(listenfd);
            // Đảm bảo các client khác nhau có số thứ tự câu hỏi và đáp án khác nhau
            srand(time(NULL) ^ getpid());
            handle_cli(connfd);
            _exit(0); //Tiến trình con kết thúc khi thực hiện xong nhiệm vụ
        }

        close(connfd);
    }

    return 0;
}

void shuffle(int arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);

        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void handle_cli(int sockfd) {
    char buf[MAXLINE], ans[8];
    int cnt = 0;
    int arr1[10] = {0,1,2,3,4,5,6,7,8,9};
    int arr2[4] = {0,1,2,3};
    shuffle(arr1, 10);
    
    snprintf(buf, MAXLINE, "Chon 1 trong cac dap an A, B, C, D!");
    send(sockfd, buf, strlen(buf)+1, 0);

    for (int i = 0; i < 10; ++i) {
        shuffle(arr2, 4);
        struct Question *q = &questions[arr1[i]];
        // Sinh ra một câu hỏi ngẫu nhiên với đáp án được sắp xếp ngẫu nhiên
        snprintf(buf, MAXLINE, "C%d: %s\nA. %s\nB. %s\nC. %s\nD. %s",
            ++cnt, q->question, q->options[arr2[0]], q->options[arr2[1]], q->options[arr2[2]], q->options[arr2[3]]);
        // Gửi câu hỏi cho client
        send(sockfd, buf, strlen(buf)+1, 0);
        // Đọc đáp án trả lời
        read(sockfd, ans, 8);
        if (strcmp(q->correct, q->options[arr2[ans[0]-'A']]) == 0)
            snprintf(buf, MAXLINE, "Ban da chon dap an dung!");
        else
            snprintf(buf, MAXLINE, "Dap an dung la: %s", q->correct);
        // Gửi thông báo trả lời đúng sai
        send(sockfd, buf, strlen(buf)+1, 0);
        sleep(1);
    }
}
