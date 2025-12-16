#include "../../includes/Command.hpp"

Invite::Invite() : Command("INVITE") {
    std::cout << "INVITE command initialized." << std::endl
    << "Invites a user to join a specified channel." << std::endl; // To be deleted
}

Invite::~Invite() {
    std::cout << "INVITE command destroyed." << std::endl; // To be deleted
}