#include <iostream>
#include <thread>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>

using namespace std;

#define PORT 8080
#define BUFFER_SIZE 1024

int clients[2];
char board[3][3];
char symbols[2] = {'X', 'O'};
int currentPlayer = 0;
bool gameOver = false;

void initializeBoard() {
    char c = '1';
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = c++;
}

string getBoard() {
    string s;
    for (int i = 0; i < 3; i++) {
        s += "\n ";
        for (int j = 0; j < 3; j++) {
            s += board[i][j];
            if (j < 2) s += " | ";
        }
        if (i < 2) s += "\n-----------";
    }
    return s;
}

bool isWin(char symbol) {
    for (int i = 0; i < 3; i++)
        if ((board[i][0] == symbol && board[i][1] == symbol && board[i][2] == symbol) ||
            (board[0][i] == symbol && board[1][i] == symbol && board[2][i] == symbol))
            return true;

    if ((board[0][0] == symbol && board[1][1] == symbol && board[2][2] == symbol) ||
        (board[0][2] == symbol && board[1][1] == symbol && board[2][0] == symbol))
        return true;

    return false;
}

bool isDraw() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] != 'X' && board[i][j] != 'O')
                return false;
    return true;
}

void sendToAll(string message) {
    for (int i = 0; i < 2; i++)
        send(clients[i], message.c_str(), message.size(), 0);
}

void handleGame() {
    initializeBoard();
    srand(time(0));
    currentPlayer = rand() % 2;

    send(clients[currentPlayer], "You won the toss! You go first.\n", 35, 0);
    send(clients[1 - currentPlayer], "You lost the toss. Wait for your turn.\n", 40, 0);

    while (!gameOver) {
        sendToAll(getBoard());

        string prompt = "\nPlayer " + string(1, symbols[currentPlayer]) + ", enter your move (1-9): ";
        send(clients[currentPlayer], prompt.c_str(), prompt.size(), 0);

        char buffer[BUFFER_SIZE] = {0};
        read(clients[currentPlayer], buffer, BUFFER_SIZE);
        int move = buffer[0] - '1';

        int row = move / 3, col = move % 3;

        if (move < 0 || move > 8 || board[row][col] == 'X' || board[row][col] == 'O') {
            string err = "Invalid move. Try again.\n";
            send(clients[currentPlayer], err.c_str(), err.size(), 0);
            continue;
        }

        board[row][col] = symbols[currentPlayer];

        if (isWin(symbols[currentPlayer])) {
            sendToAll(getBoard());
            string winMsg = "\nPlayer " + string(1, symbols[currentPlayer]) + " wins! Game Over.\n";
            sendToAll(winMsg);
            gameOver = true;
        } else if (isDraw()) {
            sendToAll(getBoard());
            string drawMsg = "\nIt's a draw! Game Over.\n";
            sendToAll(drawMsg);
            gameOver = true;
        } else {
            currentPlayer = 1 - currentPlayer;
        }
    }

    for (int i = 0; i < 2; i++) close(clients[i]);
}

int main() {
    int server_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 2);

    cout << "Server started. Waiting for 2 players...\n";

    for (int i = 0; i < 2; i++) {
        clients[i] = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        string msg = "You are Player ";
        msg += symbols[i];
        msg += "\nWaiting for opponent...\n";
        send(clients[i], msg.c_str(), msg.size(), 0);
    }

    cout << "Both players connected. Starting game...\n";
    handleGame();

    close(server_fd);
    return 0;
}
