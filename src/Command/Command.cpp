#include "../../includes/Command.hpp"

Command::Command(const std::string& name) : _name(name) {
    std::cout << "Command " << _name << " initialized." << std::endl; // To be deleted
}

Command::~Command() {
    std::cout << "Command " << _name << " destroyed." << std::endl; // To be deleted
}
