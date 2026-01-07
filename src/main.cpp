#include <iostream>
#include <string>
#include <csignal>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>

#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Signal.hpp"

static void printUsage() {
	std::cerr << "To run the server, use: ./ircserv <port> <password>" << std::endl;
}

static bool isDigitsOnly(const char *s) {
    size_t i;

    if (!s || s[0] == '\0')
        return false;
    i = 0;
    while (s[i]) {
        if (s[i] < '0' || s[i] > '9')
            return false;
        i++;
    }
    return true;
}

static bool parsePort(const char *s, int &port, std::string &err) {
	char *end;
    long v;

    if (!isDigitsOnly(s)) {
        err = "Port must be a number (digits only).";
        return false;
    }
    errno = 0;
    end = 0;
    v = std::strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        err = "Port is not a valid integer.";
        return false;
    }
    if (v < 1 || v > 65535) {
        err = "Port must be in range 1..65535.";
        return false;
    }
    port = static_cast<int>(v);
    return true;
}

static bool validatePassword(const char *s, std::string &err) {
    if (!s || s[0] == '\0') {
        err = "Password must not be empty.";
        return false;
    }

    size_t i = 0;
    while (s[i]) {
        if (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r') {
            err = "Password must not contain whitespace.";
            return false;
        }
        i++;
    }

    return true;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage();
        return 1;
    }
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

	int port;
	const char *password;
	std::string err;
	if (!parsePort(argv[1], port, err)) {
		std::cerr << "Error: " << err << std::endl;
        printUsage();
        return 1;
	}
    password = argv[2];
    if (!validatePassword(password, err)) {
        std::cerr << "Error: " << err << std::endl;
        printUsage();
        return 1;
    }
    std::cout << "Starting server on port: " << port << std::endl;
	Server server(port, password);
    server.run();
    return 0;
}
