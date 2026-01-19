#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Notice::Notice() : Command("NOTICE") {}
Notice::~Notice() {}

static std::string buildPrefixNotice(const Client &c)
{
    std::string user = c.getName().empty() ? "user" : c.getName();
    std::string host = c.getIp().empty() ? "localhost" : c.getIp();
    return c.getNickname() + "!" + user + "@" + host;
}

void Notice::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    (void)fd;
    if (msg.params.size() < 2)
        return;
    int targetFd = server.findClientFdByNick(msg.params[0]);
    if (targetFd < 0)
        return;
    server.sendToClient(targetFd, ":" + buildPrefixNotice(client) + " NOTICE " + msg.params[0] + " :" + msg.params[1]);
}
