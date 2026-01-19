#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Pong::Pong() : Command("PONG") {}
Pong::~Pong() {}

bool Pong::requiresRegistration() const { return false; }

void Pong::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    (void)server; (void)fd; (void)client; (void)msg;
}
