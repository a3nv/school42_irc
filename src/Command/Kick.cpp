#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Kick::Kick() : Command("KICK") {}
Kick::~Kick() {}

void Kick::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (msg.params.size() < 2) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "KICK");
        return;
    }
    std::string reason;
    if (msg.params.size() >= 3)
        reason = msg.params[2];
    server.channelKick(fd, client, msg.params[0], msg.params[1], reason);
}
