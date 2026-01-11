#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"

Ping::Ping() : Command("PING") {
    std::cout << "PING command initialized." << std::endl
    << "Sends a PING message to check if the client is still connected." << std::endl; // To be deleted
}

Ping::~Ping() {
    std::cout << "PING command destroyed." << std::endl; // To be deleted
}

bool Ping::requiresRegistration() const
{
    return false;
}

void Ping::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    std::string token;
    (void)client;

    if (msg.params.empty()) {
        server.sendToClient(fd, ":irc42 409 :No origin specified");
        return;
    }
    token = msg.params[0];

    server.sendToClient(fd, "PONG :"+ token);
}