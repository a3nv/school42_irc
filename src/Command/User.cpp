#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"

User::User() : Command("USER") {
    std::cout << "USER command initialized." << std::endl
    << "Registers a new user with the server." << std::endl; // To be deleted
}

User::~User() {
    std::cout << "USER command destroyed." << std::endl; // To be deleted
}

bool User::execute(Server& server, IrcMessage& message, int fd) {
    if (message.params.size() < 4) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "USER");
        return false;
    }
    
    return true;
}