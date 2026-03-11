#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")
#define PORT 8888

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr = {AF_INET, htons(PORT)};
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Dang ket noi den server...\n");
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Ket noi THAT BAI! Hay mo Server truoc.\n");
        return -1;
    }

    char buffer[2048] = {0};
    recv(sock, buffer, 1024, 0); 
    printf("Server: %s\n", buffer);

    printf("Nhap fleet (VD: B:A1,A2,A3,A4 P1:B1,B2 P2:C1,C2): ");
    fflush(stdout); 
    char placement[100];
    fgets(placement, 100, stdin);
    send(sock, placement, strlen(placement), 0);

    while (1) {
        memset(buffer, 0, 2048);
        int len = recv(sock, buffer, 2048, 0);
        if (len <= 0) break;

        if (strcmp(buffer, "TURN") == 0) {
            printf("\n--- DEN LUOT BAN --- \nNhap toa do ban (VD: A1): ");
            fflush(stdout);
            char move[10];
            scanf("%s", move);
            send(sock, move, strlen(move), 0);
        } else if (strcmp(buffer, "WAIT") == 0) {
            printf("Dang cho doi thu...\n");
        } else if (strcmp(buffer, "WIN") == 0) {
            printf("\nCHUC MUNG! BAN DA THANG!\n"); break;
        } else if (strcmp(buffer, "LOSE") == 0) {
            printf("\nRAT TIEC! BAN DA THUA!\n"); break;
        } else {
            printf("%s\n", buffer);
        }
    }
    closesocket(sock);
    WSACleanup();
    system("pause"); // Giữ màn hình sau khi kết thúc
    return 0;
}