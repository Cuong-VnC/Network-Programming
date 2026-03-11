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

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server = {AF_INET, htons(PORT), INADDR_ANY};
    
    bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    listen(server_fd, 2);

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

    int turn = 0;
    while (1) {
        int opp = 1 - turn;
        send_game_view(turn);
        send(client_sockets[turn], "TURN", 4, 0);
        send(client_sockets[opp], "WAIT", 4, 0);

        char move[10] = {0};
        recv(client_sockets[turn], move, 10, 0);
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
        if (players[opp].health[1] + players[opp].health[2] + players[opp].health[3] == 0) {
            send(client_sockets[turn], "WIN", 3, 0);
            send(client_sockets[opp], "LOSE", 4, 0);
            break;
        }
        turn = opp;
    }
    WSACleanup();
    return 0;
}