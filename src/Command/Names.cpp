#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Names::Names() : Command("NAMES") {}
Names::~Names() {}

void Names::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (msg.params.empty()) {
        return;
    }

    std::string s = msg.params[0];
    size_t i = 0;
    while (i <= s.size()) {
        size_t j = s.find(',', i);
        if (j == std::string::npos) j = s.size();
        std::string chan = s.substr(i, j - i);
        if (!chan.empty())
            server.sendNames(fd, client, chan);
        i = j + 1;
    }
}
