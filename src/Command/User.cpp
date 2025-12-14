#include "../includes/Command.hpp"

User::User() : Command("USER") {
    std::cout << "USER command initialized." << std::endl
    << "Registers a new user with the server." << std::endl; // To be deleted
}

User::~User() {
    std::cout << "USER command destroyed." << std::endl; // To be deleted
}