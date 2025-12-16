#include "../../includes/Command.hpp"

Mode::Mode() : Command("MODE") {
    std::cout << "MODE command initialized." << std::endl
    << "Sets or unsets user or channel modes." << std::endl; // To be deleted
}

Mode::~Mode() {
    std::cout << "MODE command destroyed." << std::endl; // To be deleted
}