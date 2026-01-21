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
    std::string target;
    std::string text;

    if (msg.params.empty()) {
        server.sendError(fd, ERR_NORECIPIENT, "PRIVMSG");
        return;
    }
    if (msg.params.size() < 2 || msg.params[1].empty()) {
        server.sendError(fd, ERR_NOTEXTTOSEND, "");
        return;
    }
    target = msg.params[0];
    text = msg.params[1];

    if (!target.empty() && target[0] == '#') {
        std::string key;
        std::string line;

        key = Server::normalizeChanKey(target);
        if (!server.channelExists(key)) {
            server.sendError(fd, ERR_NOSUCHCHANNEL, target);
            return;
        }
        if (!server.isClientInChannel(key, fd)) {
            server.sendError(fd, ERR_CANNOTSENDTOCHAN, target);
            return;
        }
        line = ":" + buildPrefix(client) + " PRIVMSG " + server.getChannelDisplayName(key) + " :" + text;
        server.sendToChannel(key, line, fd);
        return;
    }

    int targetFd = server.findClientFdByNick(target);
    if (targetFd < 0) {
        server.sendError(fd, ERR_NOSUCHNICK, target);
        return;
    }
    server.sendToClient(targetFd, ":" + buildPrefix(client) + " PRIVMSG " + target + " :" + text);
}
