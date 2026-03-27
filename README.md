### 🚢 C-Battleship: Networked Console Combat
A classic Battleship game built with C using Socket Programming. This project features a client-server architecture, allowing two players to engage in a strategic naval battle over a local network or a single machine via Terminal.

### 📌 Project Overview
The game follows a 5x5 grid system where players must strategically place their fleet and take turns to destroy the opponent's ships. This implementation focuses on robust network communication, handling win/loss conditions, and real-time board updates.

### ✨ Key Features
Client-Server Architecture: Centralized server logic to handle game state and prevent cheating.

Cross-Platform Design: Specifically optimized for Windows (Winsock2)

Fleet Management: - 1x Battleship (4 cells)
    2x Patrol Boats (2 cells each)

Advanced Game Logic: - Dynamic board rendering after every turn.
    "Sunk" notification when all cells of a specific ship are destroyed.
    Turn-based synchronization (prevents playing out of order).

### 🛠 System Requirements & Installation
1. Requirements
- OS: Windows 10/11 (Recommended).
- Compiler: GCC (via MinGW-w64 for Windows).
- Library: ws2_32 (Windows Socket library).

2. Compilation
Open your terminal (CMD or PowerShell) in the project directory and run the following commands:
# Compile the Server
    gcc server.c -o server -lws2_32
# Compile the Client
    gcc client.c -o client -lws2_32
# Compile the Spectator
    gcc spectator.c -o spectator -lws2_32

### 🚀 How to Play
To start a game, you need to open three separate terminal windows.
- Step 1: Initialize the Server
    In the first terminal, run:
  #
        server.exe
The server will start listening on port 8888 and wait for two players to connect.
- Step 2: Connect Clients
    In the second and third terminals, run:
  #
        client.exe
- Step 3: Ship Placement
Once connected, both players must input their fleet coordinates using the following format(B = Battleship, P1/P2 = Patrol Boats):
#
        B:A1,A2,A3,A4 P1:B1,B2 P2:C1,C2
- Step 4: Join as a Spectator (Optional)
Important: Spectators can only join after both players have finished placing their ships.
Open a new terminal (on the same machine or another in the LAN) and run:
#
        spectator.exe
Enter the Server's IP address when prompted (enter 127.0.0.1 if running on the same machine).
- Step 5: Battle
Players take turns entering target coordinates.
Legend:
+ [X]: Hit (You struck an enemy ship).
+ [O]: Miss (You hit the water).
+ [?]: Alert (Your own ship has been hit).
+ [S]: Ship (Visible only in the Spectator view).
### 📡 Communication Protocol
The game utilizes a custom string-based protocol for data exchange:
#
Command      |  Direction  | Description
| :---:  | :--- | :--- |
CONNECT_OK   |   S ➔ C    | Confirms the client is successfully connected.
TURN         |   S ➔ C    |Notifies the client it is their turn to shoot.
WAIT         |   S ➔ C    |Notifies the client to wait for the opponent.
HIT/MISS/SUNK|   S ➔ C    |Reports the result of the last shot.
WIN/LOSE     |   S ➔ C    |Declares the end of the game and the winner.
CLEAR_SCREEN |   S ➔ Spec |Signals the spectator client to clear the terminal for real-time frame rendering.
### 🔧 Troubleshooting
- Connection Timed Out: Ensure your Windows Firewall is not blocking port 8888.
- GCC not recognized: Add your MinGW bin folder to the System Environment Variables (Path).
- Frozen Screen: Ensure you start the server first before launching the client.
