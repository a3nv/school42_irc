#include "../../includes/Command.hpp"

Pong::Pong() : Command("PONG") {
    std::cout << "PONG command initialized." << std::endl
    << "Response to a PING message to keep the connection alive." << std::endl; // To be deleted
}

Pong::~Pong() {
    std::cout << "PONG command destroyed." << std::endl; // To be deleted
}