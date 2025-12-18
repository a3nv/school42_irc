#include "../includes/Server.hpp"

#include <iostream>
#include <stdexcept>
#include <cstring>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

Server::Server(int port, const std::string& password)
: _port(port), _password(password), _listenFd(-1) {
}

Server::~Server() {
    if (_listenFd != -1)
        close(_listenFd);
}

void Server::run() {
    sockaddr_in addr;
    int opt;

    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd < 0)
        throw std::runtime_error("socket() failed");

    opt = 1;
    if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");

    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<unsigned short>(_port));

    if (bind(_listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        throw std::runtime_error("bind() failed");

    if (listen(_listenFd, 10) < 0)
        throw std::runtime_error("listen() failed");

    std::cout << "Listening on port " << _port << std::endl;

    while (true) {
        sockaddr_in client;
        socklen_t len;
        int cfd;

        len = sizeof(client);
        cfd = accept(_listenFd, reinterpret_cast<sockaddr*>(&client), &len);
        if (cfd < 0)
            throw std::runtime_error("accept() failed");

        std::cout << "Client connected from "
                  << inet_ntoa(client.sin_addr)
                  << ":" << ntohs(client.sin_port)
                  << std::endl;

        close(cfd);
    }
}