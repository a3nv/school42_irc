#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

PrivMsg::PrivMsg() : Command("PRIVMSG") {}
PrivMsg::~PrivMsg() {}

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

    const std::string &target = msg.params[0];
    const std::string &text = msg.params[1];

    if (server.isChannelName(target)) {
        server.channelPrivmsg(fd, client, target, text);
        return;
    }

    int targetFd = server.findClientFdByNick(target);
    if (targetFd < 0) {
        server.sendError(fd, ERR_NOSUCHNICK, target);
        return;
    }
    server.sendToClient(targetFd, ":" + client.getNickname() + "!" + (client.getName().empty() ? "user" : client.getName()) + "@" + (client.getIp().empty() ? "localhost" : client.getIp())
        + " PRIVMSG " + target + " :" + text);
}
