#include "../../includes/Command.hpp"

Command::Command(const std::string& name) : _name(name) {}
Command::~Command() {}

bool Command::requiresRegistration() const
{
    return true;
}

void Command::run(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    if (requiresRegistration()) {
        (void)client;
    }
    execute(server, fd, client, msg);
}