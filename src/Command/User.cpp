#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

User::User() : Command("USER") {}
User::~User() {}

bool User::requiresRegistration() const { return false; }

void User::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (client.isRegistered()) {
        server.sendError(fd, ERR_ALREADYREGISTRED, "");
        return;
    }
    if (msg.params.size() < 4) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "USER");
        return;
    }
    client.setName(msg.params[0]);
    client.setRealName(msg.params[3]);
    client.setHasUser(true);
    server.tryRegister(fd, client);
}
