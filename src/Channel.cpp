#include "../includes/Channel.hpp"
#include <sstream>

Channel::Channel()
: _name(""), _key(""),
  _members(), _ops(),
  _inviteOnly(false), _topicOpOnly(false),
  _keyPassword(""), _hasKey(false),
  _limit(0), _hasLimit(false),
  _invited(),
  _topic(""), _hasTopic(false)
{
}

Channel::Channel(const std::string &displayName, const std::string &key)
: _name(displayName), _key(key),
  _members(), _ops(),
  _inviteOnly(false), _topicOpOnly(false),
  _keyPassword(""), _hasKey(false),
  _limit(0), _hasLimit(false),
  _invited(),
  _topic(""), _hasTopic(false)
{
}

const std::string &Channel::name() const { return _name; }
const std::string &Channel::key() const { return _key; }

bool Channel::hasMember(int fd) const { return _members.count(fd) != 0; }
void Channel::addMember(int fd) { _members.insert(fd); }
void Channel::removeMember(int fd) { _members.erase(fd); _ops.erase(fd); _invited.erase(fd); }
const std::set<int> &Channel::members() const { return _members; }
bool Channel::empty() const { return _members.empty(); }

bool Channel::isOp(int fd) const { return _ops.count(fd) != 0; }
void Channel::addOp(int fd) { _ops.insert(fd); }
void Channel::removeOp(int fd) { _ops.erase(fd); }
const std::set<int> &Channel::ops() const { return _ops; }

bool Channel::isInvited(int fd) const { return _invited.count(fd) != 0; }
void Channel::invite(int fd) { _invited.insert(fd); }
void Channel::uninvite(int fd) { _invited.erase(fd); }

bool Channel::inviteOnly() const { return _inviteOnly; }
void Channel::setInviteOnly(bool v) { _inviteOnly = v; }

bool Channel::topicOpOnly() const { return _topicOpOnly; }
void Channel::setTopicOpOnly(bool v) { _topicOpOnly = v; }

bool Channel::hasKey() const { return _hasKey; }
const std::string &Channel::keyPassword() const { return _keyPassword; }
void Channel::setKeyPassword(const std::string &k) { _keyPassword = k; _hasKey = true; }
void Channel::clearKeyPassword() { _keyPassword.clear(); _hasKey = false; }

bool Channel::hasLimit() const { return _hasLimit; }
size_t Channel::limit() const { return _limit; }
void Channel::setLimit(size_t v) { _limit = v; _hasLimit = true; }
void Channel::clearLimit() { _limit = 0; _hasLimit = false; }

bool Channel::hasTopic() const { return _hasTopic; }
const std::string &Channel::topic() const { return _topic; }
void Channel::setTopic(const std::string &t) { _topic = t; _hasTopic = true; }
void Channel::clearTopic() { _topic.clear(); _hasTopic = false; }

std::string Channel::modeString() const
{
    std::string m = "+";
    if (_inviteOnly) m += "i";
    if (_topicOpOnly) m += "t";
    if (_hasKey) m += "k";
    if (_hasLimit) m += "l";
    return m;
}
