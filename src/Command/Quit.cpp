#include "../../includes/Command.hpp"

Quit::Quit() : Command("QUIT") {
    std::cout << "QUIT command initialized." << std::endl
    << "Allows a user to disconnect from the server." << std::endl; // To be deleted
}

Quit::~Quit() {
    std::cout << "QUIT command destroyed." << std::endl; // To be deleted
}