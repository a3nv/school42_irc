#include "../../includes/Command.hpp"

Topic::Topic() : Command("TOPIC") {
    std::cout << "TOPIC command initialized." << std::endl
    << "Sets or retrieves the topic of a specified channel." << std::endl; // To be deleted
}

Topic::~Topic() {
    std::cout << "TOPIC command destroyed." << std::endl; // To be deleted
}