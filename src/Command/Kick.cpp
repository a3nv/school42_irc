#include "../../includes/Command.hpp"

Kick::Kick() : Command("KICK") {
    std::cout << "KICK command initialized." << std::endl
    << "Removes a user from a specified channel." << std::endl; // To be deleted
}

Kick::~Kick() {
    std::cout << "KICK command destroyed." << std::endl; // To be deleted
}