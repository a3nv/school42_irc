#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

Join::Join() : Command("JOIN") {}
Join::~Join() {}

void Join::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (msg.params.empty()) {
        server.sendError(fd, ERR_NEEDMOREPARAMS, "JOIN");
        return;
    }

    std::vector<std::string> chans;
    std::vector<std::string> keys;
    {
        std::string s = msg.params[0];
        size_t i = 0;
        while (i <= s.size()) {
            size_t j = s.find(',', i);
            if (j == std::string::npos) j = s.size();
            chans.push_back(s.substr(i, j - i));
            i = j + 1;
        }
    }
    if (msg.params.size() >= 2) {
        std::string s = msg.params[1];
        size_t i = 0;
        while (i <= s.size()) {
            size_t j = s.find(',', i);
            if (j == std::string::npos) j = s.size();
            keys.push_back(s.substr(i, j - i));
            i = j + 1;
        }
    }

    for (size_t idx = 0; idx < chans.size(); ++idx) {
        std::string key = (idx < keys.size()) ? keys[idx] : "";
        server.joinChannel(fd, client, chans[idx], key);
    }
}
