#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "tictactoe.h"

#define PORT 8080
#define MAXLINE 1024

int check(int board[3][3]);
void notify_ready(int sockfd, int no);
void notify_turn(int sockfd);
void notify_state(int sockfd, int x, int y, int no);
void notify_result(int sockfd, int winner);

int main() {
    int listenfd;
    int turn = 1;
    char buf[MAXLINE];
    int connfd1, connfd2;
    struct sockaddr_in servadddr, cliaddr;
    int board[3][3]; // 0 : ô trống, 1 : người chơi 1, 2 : người chơi 2

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Lỗi tạo socket!");
        exit(EXIT_FAILURE);
    }

    memset(&cliaddr, 0, sizeof(cliaddr));
    memset(&servadddr, 0, sizeof(servadddr));
    servadddr.sin_family = AF_INET;
    servadddr.sin_port = htons(PORT);
    servadddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr*)&servadddr, sizeof(servadddr)) < 0) {
        perror("Lỗi khi bind!");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    listen(listenfd, 2);

    socklen_t clilen = sizeof(cliaddr);
    connfd1 = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
    connfd2 = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);

    notify_ready(connfd1, 1);
    notify_ready(connfd2, 2);

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) board[i][j] = 0;

    ssize_t n;
    MessageHeader header;
    for (;;) {
        int connfd = (turn == 1) ? connfd1 : connfd2;
        notify_turn(connfd);
        // Đọc header
        if ((n = recv(connfd, &header, sizeof(header), 0)) < 0) {
            perror("read error!");
            exit(EXIT_FAILURE);
        }
        // Nếu một client mất kết nối
        if (n == 0) {
            // Đóng kết nối server
            break;
        }
        // Kiểm tra header
        if (header.type != MOVE) {
            perror("Lỗi định dạng khi phân tích thông điệp client!\n");
            exit(EXIT_FAILURE);
        }
        // Lấy nước đi từ payload
        int len = header.length;
        MovePayload move;
        if ((n = recv(connfd, &move, len, 0)) < 0) {
            perror("read error!");
            exit(EXIT_FAILURE);
        }
        int x = move.x;
        int y = move.y;
        // Cập nhật trạng thái bảng
        board[x][y] = turn;
        // Thông báo trạng thái cập nhật cho người chơi
        notify_state(connfd1, x, y, turn);
        notify_state(connfd2, x, y, turn);
        // Kiểm tra trạng thái bàn cờ
        int status = check(board);
        if (status == -1) {
            turn = (turn == 1) ? 2 : 1;
        } else {
            notify_result(connfd1, status);
            notify_result(connfd2, status);
            break;
        }
    }

    close(connfd1);
    close(connfd2);
    close(listenfd);

    return 0;
}

// trả về 1 nếu người chơi 1 thắng, 2 nếu người chơi 2 thắng, 0 nếu hòa, -1 nếu chưa kết thúc
int check(int b[3][3]) {
    for (int i = 0; i < 3; i++)
        if ((b[i][0] && b[i][0]==b[i][1] && b[i][1]==b[i][2]) || (b[0][i] && b[0][i]==b[1][i] && b[1][i]==b[2][i]))
            return b[i][0] ? b[i][0] : b[0][i];

    if ((b[0][0] && b[0][0]==b[1][1] && b[1][1]==b[2][2]) || (b[0][2] && b[0][2]==b[1][1] && b[1][1]==b[2][0]))
        return b[1][1];

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (b[i][j] == 0) return -1;

    return 0;
}

void notify_ready(int sockfd, int no) {
    MessageHeader header;
    ReadyPayload payload;

    header.type = READY;
    header.length = READY_LENGTH;

    payload.no = no;

    send(sockfd, &header, sizeof(header), 0);
    send(sockfd, &payload, sizeof(payload), 0);
}

void notify_turn(int sockfd) {
    MessageHeader header;

    header.type = TURN_NOTIFICATION;
    header.length = 0;

    send(sockfd, &header, sizeof(header), 0);
}

void notify_state(int sockfd, int x, int y, int no) {
    MessageHeader header;
    StatePayload payload;

    header.type = STATE_UPDATE;
    header.length = STATE_UPDATE_LENGTH;

    payload.x = x;
    payload.y = y;
    payload.no = no;
    
    send(sockfd, &header, sizeof(header), 0);
    send(sockfd, &payload, sizeof(payload), 0);
}

void notify_result(int sockfd, int winner) {
    MessageHeader header;
    ResultPayload payload;

    header.type = RESULT;
    header.length = RESULT_LENGTH;

    payload.winner = winner;

    send(sockfd, &header, sizeof(header), 0);
    send(sockfd, &payload, sizeof(payload), 0);
}