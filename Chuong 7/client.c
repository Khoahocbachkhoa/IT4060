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

void print_board(int board[3][3]);

int main() {
    int sockfd;
    int playerno; // số thứ tự người chơi, người chơi 1 : x, người chơi 2 : o
    char buf[MAXLINE];
    struct sockaddr_in serv_addr;
    int board[3][3]; // trạng thái của bảng, được cập nhật từ server

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Lỗi tạo socket!");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) board[i][j] = 0;

    MessageHeader header;
    ssize_t n;
    for (;;) {
        if ((n = recv(sockfd, &header, sizeof(header), 0)) < 0) {
            perror("read error");
            exit(EXIT_FAILURE);
        }
        if (n == 0) {
            printf("server đóng kết nối!\n");
            break;
        }
        uint8_t type = header.type;
        if (type == READY) {
            ReadyPayload payload;
            recv(sockfd, &payload, sizeof(payload), 0);
            printf("Trò chơi sẵn sàng! Bạn là người chơi thứ %d(%c)\n", 
                payload.no, (payload.no == 1) ? 'x' : 'o');
            playerno = payload.no;
            print_board(board);
        } else if (type == TURN_NOTIFICATION) {
            printf("Chọn 1 ô để đánh: ");
            int x, y;
            scanf("%d %d", &x, &y);
            while (x < 0 || x > 2 || y < 0 || y > 2 || board[x][y] != 0) {
                printf("Không phải ô trống, hãy chọn lại!\n");
                printf("Chọn 1 ô để đánh: ");
                scanf("%d %d", &x, &y);
            }
            board[x][y] = playerno;
            // gửi ô đã chọn cho server
            header.type = MOVE;
            header.length = MOVE_LENGTH;
            MovePayload payload;
            payload.x = x;
            payload.y = y;
            send(sockfd, &header, sizeof(header), 0);
            send(sockfd, &payload, sizeof(payload), 0);
        } else if (type == STATE_UPDATE) {
            StatePayload payload;
            recv(sockfd, &payload, sizeof(payload), 0);
            // update board
            int x = payload.x;
            int y = payload.y;
            int no = payload.no;
            board[x][y] = no;
            print_board(board);
        } else if (type == RESULT) {
            ResultPayload payload;
            recv(sockfd, &payload, sizeof(payload), 0);
            int winner = payload.winner;
            if (winner == playerno)
                printf("You win!\n");
            else if (winner == 0)
                printf("Tie!\n");
            else
                printf("You lost!\n");
        }
    }

    return 0;
}

void print_board(int board[3][3]) {
    for (int i = 0; i < 3; ++i, putchar('\n'))
        for (int j = 0; j < 3; ++j)
            printf("%c", (board[i][j] == 0) ? '_' : (board[i][j] == 1 ? 'x' : 'o'));
    putchar('\n');
}