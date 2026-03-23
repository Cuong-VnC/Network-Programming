#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>     
#include <winsock2.h>  // Thư viện socket 

#pragma comment(lib, "ws2_32.lib") // Link thư viện Winsock

#define PORT 8888 // Port kết nối tới server

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa); // Khởi động Winsock

    // Tạo socket TCP
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    // Cấu hình địa chỉ server
    struct sockaddr_in serv_addr = {AF_INET, htons(PORT)};
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Kết nối localhost

    printf("Dang ket noi den server...\n");

    // Kết nối đến server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Ket noi THAT BAI! Hay mo Server truoc.\n");
        return -1;
    }

    char buffer[2048] = {0};

    // Nhận thông báo từ server (CONNECT_OK)
    recv(sock, buffer, 1024, 0); 
    printf("Server: %s\n", buffer);

    // Nhập vị trí đặt tàu
    printf("Nhap fleet (VD: B:A1,A2,A3,A4 P1:B1,B2 P2:C1,C2): ");
    fflush(stdout); 

    char placement[100];
    fgets(placement, 100, stdin); // Nhập chuỗi từ bàn phím

    send(sock, placement, strlen(placement), 0); // Gửi vị trí tàu lên server

    // Vòng lặp game
    while (1) {
        memset(buffer, 0, 2048); // Xóa buffer

        int len = recv(sock, buffer, 2048, 0); // Nhận dữ liệu từ server
        if (len <= 0) break; // Nếu mất kết nối thì thoát

        // Nếu đến lượt mình
        if (strcmp(buffer, "TURN") == 0) {
            printf("\n--- DEN LUOT BAN --- \nNhap toa do ban (VD: A1): ");
            fflush(stdout);

            char move[10];
            scanf("%s", move); // Nhập nước đi

            send(sock, move, strlen(move), 0); // Gửi nước đi
        } 
        // Nếu phải chờ
        else if (strcmp(buffer, "WAIT") == 0) {
            printf("Dang cho doi thu...\n");
        } 
        // Nếu thắng
        else if (strcmp(buffer, "WIN") == 0) {
            printf("\nCHUC MUNG! BAN DA THANG!\n"); 
            break;
        } 
        // Nếu thua
        else if (strcmp(buffer, "LOSE") == 0) {
            printf("\nRAT TIEC! BAN DA THUA!\n"); 
            break;
        } 
        // Nhận thông tin khác (board, HIT, MISS...)
        else {
            printf("%s\n", buffer);
        }
    }

    closesocket(sock); // Đóng socket
    WSACleanup();      // Giải phóng Winsock

    system("pause"); // Giữ màn hình

    return 0;
}
