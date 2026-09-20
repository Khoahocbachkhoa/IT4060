#ifndef TICTACTOE
#define TICTACTOE

#include <stdint.h>
/*
    Định dạng thông điệp đơn giản
    | type | length | payload |
    
    type: chỉ loại thông điệp
    length: độ dài phần payload tính theo byte
    payload: tùy từng loại thông điệp cụ thể
*/
#define READY 0x01
#define MOVE 0x02
#define STATE_UPDATE 0x03
#define RESULT 0x04
#define TURN_NOTIFICATION 0x05

#define READY_LENGTH           (sizeof(ReadyPayload))
#define MOVE_LENGTH            (sizeof(MovePayload))
#define STATE_UPDATE_LENGTH    (sizeof(StatePayload))
#define RESULT_LENGTH          (sizeof(ResultPayload))

typedef struct {
    uint8_t  type;
    uint32_t length;
} MessageHeader; /* Tiêu đề thông điệp */

typedef struct {
    int8_t x;
    int8_t y;
} MovePayload; /* Thông báo nước đi cho server */

typedef struct {
    int8_t no;
} ReadyPayload; /* Thông báo trò chơi bắt đầu + số thứ tự người chơi */

typedef struct {
    int8_t x;
    int8_t y;
    int8_t no;
} StatePayload; /* Thông báo cập nhật bảng cho người chơi */

typedef struct {
    int8_t winner;
} ResultPayload; /* Thông báo kết quả trò chơi */

#endif