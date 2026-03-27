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

    char server_ip[20];
    printf("Nhap dia chi IP cua Server (De trong/Nhap 127.0.0.1 neu chay tren cung may): ");
    scanf("%19s", server_ip);
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Ket noi THAT BAI! Hay kiem tra IP hoac mo Server truoc.\n");
        system("pause");
        return -1;
    }

    // Xoá dòng chữ kết nối ban đầu để chuẩn bị khung hình
    system("cls");

    char buffer[2048];
    while (1) {
        memset(buffer, 0, 2048);
        int len = recv(sock, buffer, 2048, 0);
        if (len <= 0) {
            printf("\nServer da dong ket noi hoac tran dau ket thuc.\n");
            break;
        }

        // Bắt tín hiệu làm mới khung hình (Realtime rendering)
        if (strncmp(buffer, "CLEAR_SCREEN\n", 13) == 0) {
            system("cls"); // Lệnh xoá màn hình Console
            printf("%s", buffer + 13); // Bỏ qua 13 ký tự đầu tiên
        } else {
            printf("%s", buffer);
        }
    }

    closesocket(sock);
    WSACleanup();
    system("pause");
    return 0;
}