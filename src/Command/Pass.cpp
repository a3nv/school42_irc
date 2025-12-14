#include "../includes/Command.hpp"

Pass::Pass() : Command("PASS") {
    std::cout << "PASS command initialized: " << std::endl 
    << "Authenticates the client with the server password before registration." << std::endl; // To be deleted
}

Pass::~Pass() {
    std::cout << "PASS command destroyed." << std::endl; // To be deleted
}