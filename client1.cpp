#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
using namespace std;

void receive(int sock) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int val = read(sock, buffer, sizeof(buffer));
        if (val > 0) {
            cout << "\nMessage: " << buffer << "\n> ";
            cout.flush();
        } else {
            cout << "\nDisconnected from server.\n";
            break;
        }
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Socket creation failed\n";
        return 1;
    }

    sockaddr_in serv_addr = {};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9999);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection failed\n";
        return 1;
    }

    thread t(receive, sock);
    string msg;

    while (true) {
        cout << "> ";
        getline(cin, msg);
        if (msg == "exit") break;
        send(sock, msg.c_str(), msg.length(), 0);
    }

    close(sock);
    t.join();
    return 0;
}
