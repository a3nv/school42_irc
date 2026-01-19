#include "../includes/Client.hpp"
#include <cstddef>
#include <string>
#include <unistd.h>

Client::Client()
    : _fd(-1),
      _port(-1),
      _ip(""),
      _name(""),
      _realname(""),
      _nickname(""),
      _inbuf(""),
      _hasPass(false),
      _hasNick(false),
      _hasUser(false),
      _registered(false)
{
}

Client::Client(int fd, int port, const std::string& ip)
    : _fd(fd),
      _port(port),
      _ip(ip),
      _name(""),
      _realname(""),
      _nickname(""),
      _inbuf(""),
      _hasPass(false),
      _hasNick(false),
      _hasUser(false),
      _registered(false)
{
}

Client::~Client() {}

void Client::appendInput(const char* data, size_t n) {
	_inbuf.append(data, n);
}

bool Client::extractLine(std::string &line) {
	std::string::size_type pos = _inbuf.find('\n');
	if (pos == std::string::npos)
		return false;
	line = _inbuf.substr(0, pos);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	_inbuf.erase(0, pos + 1);
	return true;
}

int Client::getFd() const { return _fd; }
int Client::getPort() const { return _port; }
std::string Client::getIp() const { return _ip; }

void Client::setName(const std::string& name) { _name = name; }
std::string Client::getName() const { return _name; }

void Client::setRealName(const std::string& realname) { _realname = realname; }
std::string Client::getRealName() const { return _realname; }

void Client::setNickname(const std::string& nickname) { _nickname = nickname; }
std::string Client::getNickname() const { return _nickname; }

void Client::setHasPass(bool v) { _hasPass = v; }
bool Client::hasPass() const { return _hasPass; }

void Client::setHasNick(bool v) { _hasNick = v; }
bool Client::hasNick() const { return _hasNick; }

void Client::setHasUser(bool v) { _hasUser = v; }
bool Client::hasUser() const { return _hasUser; }

void Client::setRegistered(bool v) { _registered = v; }
bool Client::isRegistered() const { return _registered; }

size_t Client::inbufSize() const { return _inbuf.size(); }
