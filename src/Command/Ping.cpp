#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Ping::Ping() : Command("PING") {}
Ping::~Ping() {}

bool Ping::requiresRegistration() const { return false; }

void Ping::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    (void)client;
    if (msg.params.empty()) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "PING");
        return;
    }
    server.sendToClient(fd, ":" + server.getServerName() + " PONG " + server.getServerName() + " :" + msg.params[0]);
}
