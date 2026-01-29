#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Cap::Cap() : Command("CAP") {}
Cap::~Cap() {}

bool Cap::requiresRegistration() const { return false; }

void Cap::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    (void)client;
    if (msg.params.empty())
        return;
    if (msg.params[0] == "LS") {
        std::string nick = client.getNickname().empty() ? "*" : client.getNickname();
        server.sendToClient(fd, ":" + server.getServerName() + " CAP " + nick + " LS :");
    }
}
