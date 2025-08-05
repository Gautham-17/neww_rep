#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
using namespace std;

char board[3][3];
int turn = 0;

void reset_board() {
    char c = '1';
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = c++;
}

void send_board(int p1, int p2) {
    string msg = "\n";
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++)
            msg += board[i][j], msg += " ";
        msg += "\n";
    }
    send(p1, msg.c_str(), msg.size(), 0);
    send(p2, msg.c_str(), msg.size(), 0);
}

bool win_check(char sym) {
    for (int i = 0; i < 3; i++)
        if ((board[i][0]==sym && board[i][1]==sym && board[i][2]==sym) ||
            (board[0][i]==sym && board[1][i]==sym && board[2][i]==sym))
            return true;
    return (board[0][0]==sym && board[1][1]==sym && board[2][2]==sym) ||
           (board[0][2]==sym && board[1][1]==sym && board[2][0]==sym);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(7000);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 2);
    cout << "Waiting for 2 players...\n";

    int p1 = accept(server_fd, nullptr, nullptr);
    int p2 = accept(server_fd, nullptr, nullptr);

    reset_board();
    char symbols[2] = {'X', 'O'};
    char buffer[2];

    while (true) {
        int player = turn % 2;
        int sock = (player == 0) ? p1 : p2;

        send_board(p1, p2);
        string msg = "Player " + to_string(player+1) + " (" + symbols[player] + "), enter (1-9): ";
        send(sock, msg.c_str(), msg.size(), 0);

        memset(buffer, 0, 2);
        read(sock, buffer, 1);
        char move = buffer[0];

        // Apply move
        bool valid = false;
        for (int i = 0; i < 3 && !valid; i++) {
            for (int j = 0; j < 3; j++) {
                if (board[i][j] == move) {
                    board[i][j] = symbols[player];
                    valid = true;
                    break;
                }
            }
        }

        if (!valid) continue;

        if (win_check(symbols[player])) {
            send_board(p1, p2);
            string win = "Player " + to_string(player + 1) + " wins!\n";
            send(p1, win.c_str(), win.size(), 0);
            send(p2, win.c_str(), win.size(), 0);
            break;
        }

        turn++;
        if (turn == 9) {
            send_board(p1, p2);
            string draw = "It's a draw!\n";
            send(p1, draw.c_str(), draw.size(), 0);
            send(p2, draw.c_str(), draw.size(), 0);
            break;
        }
    }

    close(p1); close(p2); close(server_fd);
    return 0;
}
