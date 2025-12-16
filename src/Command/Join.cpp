#include "../../includes/Command.hpp"

Join::Join() : Command("JOIN") {
    std::cout << "JOIN command initialized." << std::endl
    << "Allows a user to join a specified channel." << std::endl; // To be deleted
}

Join::~Join() {
    std::cout << "JOIN command destroyed." << std::endl; // To be deleted
}