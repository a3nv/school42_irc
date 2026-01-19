#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

PrivMsg::PrivMsg() : Command("PRIVMSG") {}
PrivMsg::~PrivMsg() {}

static std::string buildPrefix(const Client &c)
{
    std::string user = c.getName().empty() ? "user" : c.getName();
    std::string host = c.getIp().empty() ? "localhost" : c.getIp();
    return c.getNickname() + "!" + user + "@" + host;
}

void PrivMsg::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (msg.params.empty()) {
        server.sendError(fd, ERR_NORECIPIENT, "PRIVMSG");
        return;
    }
    if (msg.params.size() < 2) {
        server.sendError(fd, ERR_NOTEXTTOSEND, "");
        return;
    }
    int targetFd = server.findClientFdByNick(msg.params[0]);
    if (targetFd < 0) {
        server.sendError(fd, ERR_NOSUCHNICK, msg.params[0]);
        return;
    }
    server.sendToClient(targetFd, ":" + buildPrefix(client) + " PRIVMSG " + msg.params[0] + " :" + msg.params[1]);
}
