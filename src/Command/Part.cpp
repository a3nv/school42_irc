#include "../../includes/Command.hpp"

Part::Part() : Command("PART") {
    std::cout << "PART command initialized." << std::endl
    << "Allows a user to leave a specified channel." << std::endl; // To be deleted
}
Part::~Part() {
    std::cout << "PART command destroyed." << std::endl; // To be deleted
}