#ifndef CLIENT_CPP
#define CLIENT_CPP
#include "../includes/Client.hpp"
#include <iostream>
#include <unistd.h>

Client::Client(int fd, int port, const std::string& ip) : _fd(fd), _port(port), _ip(ip) {
    std::cout << "Client initialized. fd is: " << _fd << std::endl; // To be deleted
}
Client::~Client() {
    std::cout << "Client destroyed. fd is: " << _fd << std::endl; // To be deleted
}

void Client::setName(const std::string& name) {_name = name;}
void Client::setNickname(const std::string& nickname) {_nickname = nickname;}

std::string Client::getName() const { return _name;}
std::string Client::getNickname() const { return _nickname;}
std::string Client::getIp() const { return _ip;}
int Client::getPort() const { return _port;}
int Client::getFd() const { return _fd;}
#endif