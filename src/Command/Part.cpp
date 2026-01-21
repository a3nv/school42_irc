#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

static std::string buildPrefix(const Client &c)
{
    std::string user;
    std::string host;

    user = c.getName();
    if (user.empty())
        user = "user";
    host = c.getIp();
    if (host.empty())
        host = "localhost";
    return c.getNickname() + "!" + user + "@" + host;
}

static std::string firstFromCommaList(const std::string &s)
{
    size_t pos;

    pos = s.find(',');
    if (pos == std::string::npos)
        return s;
    return s.substr(0, pos);
}

Part::Part() : Command("PART") {}
Part::~Part() {}

void Part::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    std::string chan;
    std::string key;
    std::string partLine;

    if (msg.params.empty()) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "PART");
        return;
    }
    chan = firstFromCommaList(msg.params[0]);
    if (chan.empty() || chan[0] != '#') {
        server.sendError(fd, ERR_NOSUCHCHANNEL, chan);
        return;
    }

    key = Server::normalizeChanKey(chan);
    if (!server.isClientInChannel(key, fd)) {
        server.sendError(fd, ERR_NOTONCHANNEL, server.getChannelDisplayNameOrRaw(key, chan));
        return;
    }

    partLine = ":" + buildPrefix(client) + " PART " + server.getChannelDisplayName(key);
    server.sendToChannel(key, partLine, -1);

    server.partChannel(key, fd);
    client.partChannel(key);
}
