#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <stack>
#include <sstream>

#define PORT 8080
#define MAX 1024

using namespace std;

// Function to evaluate a postfix expression
float evaluatePostfix(const string& expr) {
    stack<float> s;
    istringstream iss(expr);
    string token;

    while (iss >> token) {
        if (isdigit(token[0]) || (token.length() > 1 && token[0] == '-')) {
            s.push(stof(token)); // convert string to float
        } else if (token == "+" || token == "-" || token == "*" || token == "/") {
            float b = s.top(); s.pop();
            float a = s.top(); s.pop();
            if (token == "+") s.push(a + b);
            else if (token == "-") s.push(a - b);
            else if (token == "*") s.push(a * b);
            else if (token == "/") s.push(a / b);
        }
    }
    return s.top();
}

int main() {
    int sockfd, newsockfd;
    char buffer[MAX];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); // create TCP socket
    if (sockfd < 0) {
        cerr << "Socket creation failed\n";
        exit(1);
    }

    // Set up server address
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY; // accept from any address
    servaddr.sin_port = htons(PORT);       // use port 8080

    // Bind socket to IP and port
    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(sockfd, 5); // listen for up to 5 clients

    cout << "Server is listening on port " << PORT << "...\n";

    while (true) {
        len = sizeof(cliaddr);
        newsockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len); // accept new client
        if (newsockfd < 0) {
            cerr << "Server accept failed\n";
            continue;
        }

        // Receive expressions and send back results
        while (true) {
            memset(buffer, 0, MAX);
            int n = read(newsockfd, buffer, MAX);
            if (n <= 0) break;

            string expr(buffer);
            if (expr == "exit") break;

            float result = evaluatePostfix(expr);
            string result_str = to_string(result);
            write(newsockfd, result_str.c_str(), result_str.size());
        }

        close(newsockfd); // close client socket
    }

    close(sockfd); // close server socket
    return 0;
}
