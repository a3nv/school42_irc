#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Invite::Invite() : Command("INVITE") {}
Invite::~Invite() {}

void Invite::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (msg.params.size() < 2) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "INVITE");
        return;
    }
    server.channelInvite(fd, client, msg.params[0], msg.params[1]);
}
