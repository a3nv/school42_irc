#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"

Ping::Ping() : Command("PING") {}

Ping::~Ping() {}

bool Ping::requiresRegistration() const
{
    return false;
}

void Ping::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    std::string token;

    (void)client;
    if (msg.params.empty()) {
        // todo: repalce with send error ERR_NOORIGIN (409) - not yet in IrcNumeric.hpp
        server.sendToClient(fd, ":irc42 409 :No origin specified");
        return;
    }

    token = msg.params[0];
    server.sendToClient(fd, "PONG :" + token);
}