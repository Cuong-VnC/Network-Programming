# Battleship Protocol Documentation

## 1. Message Framing
Tin nhắn trao đổi là chuỗi văn bản kết thúc bằng ký tự đặc biệt hoặc độ dài cố định.

## 2. Ship Placement Format
Client gửi: `B:A1,A2,A3,A4 P1:B1,B2 P2:C1,C2`
- `B`: Battleship (4 ô)
- `P1`, `P2`: Patrol Boats (2 ô mỗi tàu)

## 3. Gameplay Commands
- `TURN`: Thông báo đến lượt bắn.
- `WAIT`: Chờ đối thủ.
- `BOARD`: Gửi dữ liệu cập nhật bản đồ để hiển thị.
- `HIT/MISS/SUNK`: Kết quả của phát bắn.