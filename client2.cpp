#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
using namespace std;

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr = {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(7000);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));

    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int val = read(sock, buffer, sizeof(buffer));
        if (val <= 0) break;

        cout << buffer;

        if (strstr(buffer, "enter")) {
            string move;
            cin >> move;
            send(sock, move.c_str(), 1, 0);
        }
    }

    close(sock);
    return 0;
}
