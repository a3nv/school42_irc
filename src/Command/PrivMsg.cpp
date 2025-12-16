#include "../../includes/Command.hpp"

PrivMsg::PrivMsg() : Command("PRIVMSG") {
    std::cout << "Command " << _name << " initialized." << std::endl
    << "Handles private messages between users." << std::endl; // To be deleted
}

PrivMsg::~PrivMsg() {
    std::cout << "Command " << _name << " destroyed." << std::endl; // To be deleted
}