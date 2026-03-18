// Khai báo thư viện
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>  //Thư viện Socket

//Thư viện Winsock chạy trên Win
#pragma comment(lib, "ws2_32.lib")

#define PORT 8888    //Khai báo cổng Server
#define SIZE 5       //Kích thước Game

typedef struct {
    char board[SIZE][SIZE];     //Bàn cờ
    int ship_types[SIZE][SIZE];    //Loại tàu tại mỗi ô
    int health[4]; //Máu từng tàu
} Player;

Player players[2]; //Số người chơi
SOCKET client_sockets[2];    //Socket của 2 người chơi

void init_player(int idx) {
    memset(players[idx].board, '0', sizeof(players[idx].board));    //Gán all Board = 0
    memset(players[idx].ship_types, 0, sizeof(players[idx].ship_types));    //Reset giá trị của tàu 
    players[idx].health[1] = 4; players[idx].health[2] = 2; players[idx].health[3] = 2;    //Gán giá trị máu
}

void parse_placement(int p_idx, char *input) {
    char *types[] = {"B:", "P1:", "P2:"};    //Khai báo các loại tàu
    int hp[] = {4, 2, 2};    //Số ô mỗi tàu
    for(int i=0; i<3; i++) {
        char *p = strstr(input, types[i]);    //Tìm vị trí chuỗi
        if(p) {
            p += strlen(types[i]);
            for(int j=0; j<hp[i]; j++) {
                int col = p[0] - 'A';    // Chuyển A->0, B->1...
                int row = p[1] - '1';    // Chuyển 1->0, 2->1...
                 // Kiểm tra hợp lệ
                if(row >=0 && row < SIZE && col >=0 && col < SIZE) {
                    players[p_idx].board[row][col] = 'S';    //Đặt tàu
                    players[p_idx].ship_types[row][col] = i + 1;    //gán laoijtàu
                }
                p += 3; //chuyển sang toạ độ tiếp theo
            }
        }
    }
}

//Gửi trạng thái cho người chơi
void send_game_view(int p_idx) {
    char buffer[2048] = "\n+-- YOUR SHOTS --+ +-- ENEMY SHIPS --+\n|   A B C D E   | |   A B C D E   |\n";
    int opp = 1 - p_idx;    //lấy thhoong tin đối thủ
    for (int i = 0; i < SIZE; i++) {
        char line[256];
        sprintf(line, "| %d ", i + 1);    //in số dòng
        // Hiển thị board đối thủ
        for (int j = 0; j < SIZE; j++) {
            char c = players[opp].board[i][j];
            strcat(line, (c == 'X') ? "[X]" : (c == 'M') ? "[O]" : " . ");
        }
        strcat(line, " | | %d ");
        // Hiển thị board của mình
        for (int j = 0; j < SIZE; j++) {
            char c = players[p_idx].board[i][j];
            strcat(line, (c == 'X') ? "[?]" : " . ");
        }
        strcat(line, " |\n");
        char formatted[256];
        sprintf(formatted, line, i+1);    // Format dòng
        strcat(buffer, formatted);    // Ghép vào buffer
    }
    strcat(buffer, "+---------------+ +---------------+\n");
    // Gửi dữ liệu cho client
    send(client_sockets[p_idx], buffer, strlen(buffer), 0);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);    // Khởi động Winsock
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);    // Tạo TCP socket
    // Cấu hình địa chỉ server
    struct sockaddr_in server = {AF_INET, htons(PORT), INADDR_ANY};
    // Gắn socket với port
    bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    // Lắng nghe tối đa 2 client
    listen(server_fd, 2);

    printf("Server dang cho 2 nguoi choi...\n");
    // Nhận kết nối từ 2 người chơi
    for (int i = 0; i < 2; i++) {
        client_sockets[i] = accept(server_fd, NULL, NULL);
        init_player(i);    //khởi tạo player
        printf("Nguoi choi %d da ket noi!\n", i + 1);
        send(client_sockets[i], "CONNECT_OK", 10, 0);    // Gửi tín hiệu OK
    }
     // Nhận dữ liệu đặt tàu
    for (int i = 0; i < 2; i++) {
        char buf[1024] = {0};
        recv(client_sockets[i], buf, 1024, 0);     // Nhận dữ liệu
        parse_placement(i, buf);    // Xử lý đặt tàu
        printf("Nguoi choi %d da dat tau xong.\n", i + 1);
    }

    int turn = 0; // Player 0 đi trước

     // Vòng lặp game
    while (1) {
        int opp = 1 - turn;     // Đối thủ
        send_game_view(turn);    // Gửi trạng thái game
        send(client_sockets[turn], "TURN", 4, 0);    // Báo đến lượt
        send(client_sockets[opp], "WAIT", 4, 0);    // Báo chờ

        char move[10] = {0};
        recv(client_sockets[turn], move, 10, 0);     // Nhận nước đi
        int c = move[0] - 'A', r = move[1] - '1';    // Cột, Hàng

        char result[128] = "Result: MISS!";    // Mặc định trượt

        // Nếu bắn trúng tàu
        if (players[opp].board[r][c] == 'S') {
            int type = players[opp].ship_types[r][c];
            players[opp].board[r][c] = 'X'; // Đánh dấu trúng
            players[opp].health[type]--;    //Trừ máu
            if (players[opp].health[type] == 0) sprintf(result, "SUNK!");     //Trìm tàu
            else strcpy(result, "HIT!");    //Trúng
        }
        // Nếu bắn trượt
        else if (players[opp].board[r][c] == '0') {
            players[opp].board[r][c] = 'M';    // Đánh dấu trượt
        }

        send(client_sockets[turn], result, strlen(result), 0);    // Gửi kết quả
        if (players[opp].health[1] + players[opp].health[2] + players[opp].health[3] == 0) {
            send(client_sockets[turn], "WIN", 3, 0);
            send(client_sockets[opp], "LOSE", 4, 0);
            break;
        }
        turn = opp;    // Đổi lượt
    }
    WSACleanup();    // Giải phóng Winsock
    return 0;
}
