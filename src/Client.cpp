#ifndef CLIENT_CPP
#define CLIENT_CPP
#include "../includes/Client.hpp"
Client::Client() : _name(""), _nickname("") {
    std::cout << "Client initialized." << std::endl; // To be deleted
}
Client::~Client() {
    std::cout << "Client destroyed." << std::endl; // To be deleted
}

void Client::setName(const std::string& name) {_name = name;}
void Client::setNickname(const std::string& nickname) {_nickname = nickname;}

std::string Client::getName() const { return _name;}
std::string Client::getNickname() const { return _nickname;}
#endif