#include <iostream>
#include <map>
#include <set>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
using namespace std;

mutex mtx;
set<int> clients;

void broadcast(const string& msg, int sender) {
    lock_guard<mutex> lock(mtx);
    for (int client : clients) {
        if (client != sender) {
            send(client, msg.c_str(), msg.length(), 0);
        }
    }
}

void handle_client(int client_socket) {
    {
        lock_guard<mutex> lock(mtx);
        clients.insert(client_socket);
    }

    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int val = read(client_socket, buffer, sizeof(buffer));
        if (val <= 0) break;  // Client disconnected
        string msg = buffer;
        broadcast(msg, client_socket);
    }

    close(client_socket);
    {
        lock_guard<mutex> lock(mtx);
        clients.erase(client_socket);
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Socket creation failed\n";
        return 1;
    }

    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(9999);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        cerr << "Bind failed\n";
        return 1;
    }

    listen(server_fd, 10);
    cout << "Chat server started on port 9999...\n";

    while (true) {
        int client_socket = accept(server_fd, NULL, NULL);
        if (client_socket >= 0) {
            thread(handle_client, client_socket).detach();
        }
    }

    close(server_fd);
    return 0;
}
