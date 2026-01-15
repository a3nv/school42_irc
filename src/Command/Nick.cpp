#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"

Nick::Nick() : Command("NICK") {
}

Nick::~Nick() {
}

bool isValidNickname(const std::string& nickname) {
    if (nickname.empty() || nickname.length() > 9)
        return false;
    if (!std::isalpha(nickname[0]))
        return false;
    for (size_t i = 1; i < nickname.length(); ++i) {
        if (!std::isalnum(nickname[i]) && nickname[i] != '-' && nickname[i] != '_')
            return false;
    }
    return true;
}

bool Nick::validate(const std::string& nickname, const Server& server, int fd) {
    if (!isValidNickname(nickname)) {
        server.sendError(fd, ERR_ERRONEUSNICKNAME, nickname);
        return false; // Invalid nickname format
    }
    if (!server.uniqueNickname(nickname, fd)) {
        server.sendError(fd, ERR_NICKNAMEINUSE, nickname);
        return false; // Nickname already in use
    }
    return true;
}

bool Nick::execute(Server& server, IrcMessage& message, int fd) {
    std::string newNick = message.params[0];
    if (!validate(newNick, server, fd)) {
        return false;
    }
    return true;
}