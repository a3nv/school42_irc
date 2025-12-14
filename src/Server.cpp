#include "../includes/Server.hpp"

Server::Server(unsigned int port, const std::string& password) : _port(port), _password(password) {
    std::cout << "Server initialized on port " << _port << " with password." << std::endl; // To be deleted
}

Server::~Server() {
    std::cout << "Server destroyed." << std::endl; // To be deleted 
}