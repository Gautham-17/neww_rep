#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX 1024

using namespace std;

int main() {
    int sockfd;
    struct sockaddr_in servaddr;
    char buffer[MAX];

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // create TCP socket
    if (sockfd < 0) {
        cerr << "Socket creation failed\n";
        return 1;
    }

    // Setup server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // localhost

    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
        cerr << "Connection with server failed\n";
        return 1;
    }

    while (true) {
        string expr;
        cout << "Enter postfix expression (or 'exit' to quit): ";
        getline(cin, expr);

        send(sockfd, expr.c_str(), expr.size(), 0); // send input to server
        if (expr == "exit") break;

        memset(buffer, 0, MAX);
        int n = read(sockfd, buffer, MAX); // receive result
        cout << "Result: " << buffer << "\n";
    }

    close(sockfd); // close client socket
    return 0;
}
