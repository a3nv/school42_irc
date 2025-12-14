#include "../includes/Command.hpp"

Nick::Nick() : Command("NICK") {
    std::cout << "NICK command initialized." << std::endl
    << "Changes or sets the nickname of a user. <---- MUST BE UNIQUE" << std::endl; // To be deleted
}

Nick::~Nick() {
    std::cout << "NICK command destroyed." << std::endl; // To be deleted
}