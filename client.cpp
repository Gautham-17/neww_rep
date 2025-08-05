#include <iostream>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

void receiveMessages(int sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(sock, buffer, BUFFER_SIZE);
        if (valread <= 0) break;
        cout << buffer << endl;

        if (strstr(buffer, "Game Over")) break;
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    thread(receiveMessages, sock).detach();

    while (true) {
        string input;
        getline(cin, input);
        if (input.size() > 0) {
            send(sock, input.c_str(), input.size(), 0);
        }
    }

    close(sock);
    return 0;
}
