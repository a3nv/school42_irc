#include "../../includes/Command.hpp"
#include "../../includes/Server.hpp"
#include "../../includes/Client.hpp"
#include "../../includes/IrcNumeric.hpp"

Command::Command(const std::string &name) : _name(name) {}

Command::~Command() {}

const std::string &Command::name() const {
    return _name;
}

bool Command::requiresRegistration() const {
    return true;
}

void Command::run(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (requiresRegistration() && !server.isRegistered(client)) {
        server.sendError(fd, ERR_NOTREGISTERED, "");
        return;
    }
    execute(server, fd, client, msg);
}