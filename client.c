#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8888
#define SIZE 5

typedef struct {
    char board[SIZE][SIZE]; 
    int ship_types[SIZE][SIZE];
    int health[4]; 
} Player;

Player players[2];
SOCKET client_sockets[2];

SOCKET spectators[10];
int num_spectators = 0;

void init_player(int idx) {
    memset(players[idx].board, '0', sizeof(players[idx].board));
    memset(players[idx].ship_types, 0, sizeof(players[idx].ship_types));
    players[idx].health[1] = 4; players[idx].health[2] = 2; players[idx].health[3] = 2;
}

void parse_placement(int p_idx, char *input) {
    char *types[] = {"B:", "P1:", "P2:"};
    int hp[] = {4, 2, 2};
    for(int i=0; i<3; i++) {
        char *p = strstr(input, types[i]);
        if(p) {
            p += strlen(types[i]);
            for(int j=0; j<hp[i]; j++) {
                int col = p[0] - 'A';
                int row = p[1] - '1';
                if(row >=0 && row < SIZE && col >=0 && col < SIZE) {
                    players[p_idx].board[row][col] = 'S';
                    players[p_idx].ship_types[row][col] = i + 1;
                }
                p += 3;
            }
        }
    }
}

void send_game_view(int p_idx) {
    char buffer[2048] = "\n+-- YOUR SHOTS --+ +-- ENEMY SHIPS --+\n|   A B C D E   | |   A B C D E   |\n";
    int opp = 1 - p_idx;
    for (int i = 0; i < SIZE; i++) {
        char line[256];
        sprintf(line, "| %d ", i + 1);
        for (int j = 0; j < SIZE; j++) {
            char c = players[opp].board[i][j];
            strcat(line, (c == 'X') ? "[X]" : (c == 'M') ? "[O]" : " . ");
        }
        strcat(line, " | | %d ");
        for (int j = 0; j < SIZE; j++) {
            char c = players[p_idx].board[i][j];
            strcat(line, (c == 'X') ? "[?]" : " . ");
        }
        strcat(line, " |\n");
        char formatted[256];
        sprintf(formatted, line, i+1);
        strcat(buffer, formatted);
    }
    strcat(buffer, "+---------------+ +---------------+\n");
    send(client_sockets[p_idx], buffer, strlen(buffer), 0);
}

// Hàm mới: Dựng khung hình realtime cho Spectator
void build_spectator_view(char* buffer, char* msg) {
    // Kèm lệnh CLEAR_SCREEN để Spectator biết phải xóa màn hình cũ
    strcpy(buffer, "CLEAR_SCREEN\n");
    if (msg) {
        strcat(buffer, msg);
        strcat(buffer, "\n");
    }
    
    strcat(buffer, "+-------------------------+ +-------------------------+\n");
    strcat(buffer, "|      A  B  C  D  E      | |      A  B  C  D  E      |\n");

    for (int i = 0; i < SIZE; i++) {
        char line[256] = "";
        char temp[32];
        
        // --- Bảng của Người chơi 1 ---
        sprintf(temp, "| %d   ", i + 1);
        strcat(line, temp);
        for (int j = 0; j < SIZE; j++) {
            char c = players[0].board[i][j];
            if (c == 'X') strcat(line, "[X]");
            else if (c == 'S') strcat(line, "[S]"); // Hiển thị tàu
            else if (c == 'M') strcat(line, "[O]");
            else strcat(line, " . ");
        }
        strcat(line, "       | ");
        
        // --- Bảng của Người chơi 2 ---
        sprintf(temp, "| %d   ", i + 1);
        strcat(line, temp);
        for (int j = 0; j < SIZE; j++) {
            char c = players[1].board[i][j];
            if (c == 'X') strcat(line, "[X]");
            else if (c == 'S') strcat(line, "[S]"); // Hiển thị tàu
            else if (c == 'M') strcat(line, "[O]");
            else strcat(line, " . ");
        }
        strcat(line, "       |\n");
        
        strcat(buffer, line);
    }
    strcat(buffer, "+-------------------------+ +-------------------------+\n");
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server = {AF_INET, htons(PORT), INADDR_ANY};
    
    bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    listen(server_fd, 10);

    printf("Server dang cho 2 nguoi choi...\n");
    for (int i = 0; i < 2; i++) {
        client_sockets[i] = accept(server_fd, NULL, NULL);
        init_player(i);
        printf("Nguoi choi %d da ket noi!\n", i + 1);
        send(client_sockets[i], "CONNECT_OK", 10, 0);
    }

    for (int i = 0; i < 2; i++) {
        char buf[1024] = {0};
        recv(client_sockets[i], buf, 1024, 0);
        parse_placement(i, buf);
        printf("Nguoi choi %d da dat tau xong.\n", i + 1);
    }

    printf("GAME BAT DAU! Nguoi xem co the vao quan sat...\n");

    int turn = 0;
    while (1) {
        int opp = 1 - turn;
        send_game_view(turn);
        send(client_sockets[turn], "TURN", 4, 0);
        send(client_sockets[opp], "WAIT", 4, 0);

        char move[10] = {0};

        while (1) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(server_fd, &readfds);
            FD_SET(client_sockets[turn], &readfds);

            select(0, &readfds, NULL, NULL, NULL);

            // Nguoi xem moi ket noi
            if (FD_ISSET(server_fd, &readfds)) {
                SOCKET spec = accept(server_fd, NULL, NULL);
                if (num_spectators < 10) {
                    spectators[num_spectators++] = spec;
                    printf("Mot Nguoi xem da tham gia!\n");
                    
                    char view_buf[2048];
                    build_spectator_view(view_buf, ">>> DANG THEO DOI TRAN DAU <<<");
                    send(spec, view_buf, strlen(view_buf), 0);
                } else {
                    closesocket(spec);
                }
            }

            // Nguoi choi danh
            if (FD_ISSET(client_sockets[turn], &readfds)) {
                recv(client_sockets[turn], move, 10, 0);
                break; 
            }
        }

        int c = move[0] - 'A', r = move[1] - '1';
        char result[128] = "Result: MISS!";
        
        if (players[opp].board[r][c] == 'S') {
            int type = players[opp].ship_types[r][c];
            players[opp].board[r][c] = 'X';
            players[opp].health[type]--;
            if (players[opp].health[type] == 0) sprintf(result, "SUNK!");
            else strcpy(result, "HIT!");
        } else if (players[opp].board[r][c] == '0') {
            players[opp].board[r][c] = 'M';
        }

        send(client_sockets[turn], result, strlen(result), 0);

        // --- Gui view Realtime cho toan bo Nguoi Xem ---
        char spec_msg[256];
        sprintf(spec_msg, ">>> Nguoi choi %d ban vao toa do %c%d: %s <<<", turn + 1, move[0], move[1], result);
        char view_buf[2048];
        build_spectator_view(view_buf, spec_msg);

        for (int i = 0; i < num_spectators; i++) {
            send(spectators[i], view_buf, strlen(view_buf), 0);
        }

        // Kiem tra thang
        if (players[opp].health[1] + players[opp].health[2] + players[opp].health[3] == 0) {
            send(client_sockets[turn], "WIN", 3, 0);
            send(client_sockets[opp], "LOSE", 4, 0);
            
            char win_msg[128];
            sprintf(win_msg, "\n=== NGUOI CHOI %d DA CHIEN THANG! ===\n", turn + 1);
            for (int i = 0; i < num_spectators; i++) {
                send(spectators[i], win_msg, strlen(win_msg), 0);
            }
            break;
        }
        turn = opp;
    }
    WSACleanup();
    return 0;
}
