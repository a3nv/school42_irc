#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Quit::Quit() : Command("QUIT") {}
Quit::~Quit() {}

bool Quit::requiresRegistration() const { return false; }

void Quit::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    (void)client; (void)msg;
    server.scheduleDisconnect(fd);
}
