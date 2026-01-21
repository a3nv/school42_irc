#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

static std::string firstFromCommaList(const std::string &s)
{
    size_t pos;

    pos = s.find(',');
    if (pos == std::string::npos)
        return s;
    return s.substr(0, pos);
}

Names::Names() : Command("NAMES") {}
Names::~Names() {}

void Names::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    std::string chan;
    std::string key;

    if (msg.params.empty()) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "NAMES");
        return;
    }
    chan = firstFromCommaList(msg.params[0]);
    if (chan.empty() || chan[0] != '#') {
        server.sendError(fd, ERR_NOSUCHCHANNEL, chan);
        return;
    }
    key = Server::normalizeChanKey(chan);
    if (!server.channelExists(key)) {
        server.sendError(fd, ERR_NOSUCHCHANNEL, chan);
        return;
    }
    server.sendNamesReply(fd, client, key);
}
