#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Pass::Pass() : Command("PASS") {}
Pass::~Pass() {}

bool Pass::requiresRegistration() const { return false; }

void Pass::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (client.isRegistered()) {
        server.sendError(fd, ERR_ALREADYREGISTRED, "");
        return;
    }
    if (msg.params.empty()) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "PASS");
        return;
    }
    if (server.passwordRequired() && msg.params[0] != server.getPassword()) {
        server.sendError(fd, ERR_PASSWDMISMATCH, "");
        return;
    }
    client.setHasPass(true);
    server.tryRegister(fd, client);
}
