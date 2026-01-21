#include "Channel.hpp"

Channel::Channel() : _name(""), _members() {}

Channel::Channel(const std::string &name) : _name(name), _members() {}

Channel::~Channel() {}

const std::string &Channel::name() const { return _name; }

void Channel::addMember(int fd)
{
    _members.insert(fd);
}

void Channel::removeMember(int fd)
{
    _members.erase(fd);
}

bool Channel::hasMember(int fd) const
{
    return _members.count(fd) != 0;
}

bool Channel::empty() const
{
    return _members.empty();
}

const std::set<int> &Channel::members() const
{
    return _members;
}
