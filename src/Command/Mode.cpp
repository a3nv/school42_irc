#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Mode::Mode() : Command("MODE") {}
Mode::~Mode() {}

void Mode::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    (void)client;
    server.channelMode(fd, client, msg);
}
