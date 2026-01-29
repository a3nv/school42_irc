#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Part::Part() : Command("PART") {}
Part::~Part() {}

void Part::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (msg.params.empty()) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "PART");
        return;
    }

    std::string partMsg;
    if (msg.params.size() >= 2)
        partMsg = msg.params[1];

    std::string s = msg.params[0];
    size_t i = 0;
    while (i <= s.size()) {
        size_t j = s.find(',', i);
        if (j == std::string::npos) j = s.size();
        std::string chan = s.substr(i, j - i);
        if (!chan.empty())
            server.partChannel(fd, client, chan, partMsg);
        i = j + 1;
    }
}
