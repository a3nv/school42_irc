#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Topic::Topic() : Command("TOPIC") {}
Topic::~Topic() {}

void Topic::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (msg.params.empty()) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "TOPIC");
        return;
    }

    const std::string &chan = msg.params[0];
    if (msg.params.size() == 1) {
        server.getChannelTopic(fd, client, chan);
        return;
    }

    server.setChannelTopic(fd, client, chan, msg.params[1]);
}
