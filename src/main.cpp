#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "To run the server, use: ./ircserv <port> <password>" << std::endl;
        return 1; // Error: Incorrect number of arguments
    }
    const char* port = argv[1];
    const char* password = argv[2];
    std::cout << "Starting server on port: " << port << " with password: " << password << std::endl;
    return 0;
}