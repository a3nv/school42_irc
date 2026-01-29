#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"

#include <cctype>

Nick::Nick() : Command("NICK") {
}

Nick::~Nick() {
}

bool Nick::requiresRegistration() const { return false; }


// nickname rules: 
// - 1..9 chars
// - first char is a letter
// - remaining chars are letters/digits or '-' or '_'
bool Nick::isValidNickname(const std::string &nickname)
{
    size_t i;
    unsigned char c;

    if (nickname.empty() || nickname.length() > 9)
        return false;

    c = static_cast<unsigned char>(nickname[0]);
    if (!std::isalpha(c))
        return false;

    i = 1;
    while (i < nickname.length()) {
        c = static_cast<unsigned char>(nickname[i]);
        if (!std::isalnum(c) && nickname[i] != '-' && nickname[i] != '_')
            return false;
        ++i;
    }
    return true;
}

bool Nick::validate(const std::string &nickname, Server &server, int fd) const
{
    if (!isValidNickname(nickname)) {
        server.sendError(fd, ERR_ERRONEUSNICKNAME, nickname);
        return false;
    }
    if (server.isNickTaken(nickname, fd)) {
        server.sendError(fd, ERR_NICKNAMEINUSE, nickname);
        return false;
    }
    return true;
}

void Nick::execute(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    std::string newNick;

    if (msg.params.empty()) {
        server.sendError(fd, ERR_NONICKNAMEGIVEN, "");
        return;
    }

    newNick = msg.params[0];
    if (!validate(newNick, server, fd))
        return;

    // todo:
    // - broadcast NICK change to channels
    // - update registration state (PASS/NICK/USER)
    client.setNickname(newNick);
	client.setHasNick(true);
	server.tryRegister(fd, client);
}
