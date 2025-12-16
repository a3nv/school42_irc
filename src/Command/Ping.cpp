#include "../../includes/Command.hpp"

Ping::Ping() : Command("PING") {
    std::cout << "PING command initialized." << std::endl
    << "Sends a PING message to check if the client is still connected." << std::endl; // To be deleted
}

Ping::~Ping() {
    std::cout << "PING command destroyed." << std::endl; // To be deleted
}