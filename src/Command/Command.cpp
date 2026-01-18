#include "../../includes/Command.hpp"

Command::Command(const std::string &name) : _name(name) {}

Command::~Command() {}

const std::string &Command::name() const
{
    return _name;
}

bool Command::requiresRegistration() const
{
    return true;
}

void Command::run(Server &server, int fd, Client &client, const IrcMessage &msg)
{
    // todo: enforce registration logic here, common checks should go here as well.
    if (requiresRegistration()) {
		std::cout << this->name() << " command requires registration" << std::endl;
    }
    execute(server, fd, client, msg);
}