#include <iostream>
#include <thread>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080
#define BUFFER_SIZE 1024

using namespace std;

map<string, int> clients;
mutex client_mutex;

void logMessage(const string& clientName, const string& message) {
    ofstream file(clientName + "_log.txt", ios::app);
    file << message << endl;
}

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE] = {0};
    read(clientSocket, buffer, BUFFER_SIZE);
    string clientName(buffer);

    {
        lock_guard<mutex> lock(client_mutex);
        clients[clientName] = clientSocket;
    }

    string welcome = "Welcome, " + clientName + "!";
    send(clientSocket, welcome.c_str(), welcome.size(), 0);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(clientSocket, buffer, BUFFER_SIZE);
        if (valread <= 0) break;

        string msg(buffer);
        size_t pos = msg.find(':');
        if (pos == string::npos) continue;

        string recipient = msg.substr(0, pos);
        string content = msg.substr(pos + 1);

        string logEntry = clientName + " -> " + recipient + ": " + content;
        logMessage(clientName, logEntry);
        logMessage(recipient, logEntry);

        lock_guard<mutex> lock(client_mutex);
        if (clients.count(recipient)) {
            send(clients[recipient], logEntry.c_str(), logEntry.size(), 0);
        }
    }

    {
        lock_guard<mutex> lock(client_mutex);
        clients.erase(clientName);
    }
    close(clientSocket);
}

void serverCommandLoop() {
    while (true) {
        string command;
        getline(cin, command);
        if (command.rfind("/logs ", 0) == 0) {
            string clientName = command.substr(6);
            ifstream file(clientName + "_log.txt");
            if (!file.is_open()) {
                cout << "No logs for client: " << clientName << endl;
                continue;
            }
            cout << "Chat logs for " << clientName << ":\n";
            string line;
            while (getline(file, line)) {
                cout << line << endl;
            }
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    cout << "Server started on port " << PORT << endl;

    thread(serverCommandLoop).detach();

    while (true) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        thread(handleClient, new_socket).detach();
    }

    return 0;
}
