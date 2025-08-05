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
        cout << "\n[Received] " << buffer << endl;
    }
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    cout << "Enter your name: ";
    string name;
    getline(cin, name);
    send(sock, name.c_str(), name.size(), 0);

    thread(receiveMessages, sock).detach();

    cout << "Send messages in format: recipient_name:message\n";
    while (true) {
        string msg;
        getline(cin, msg);
        if (msg == "/quit") break;
        send(sock, msg.c_str(), msg.size(), 0);
    }

    close(sock);
    return 0;
}
