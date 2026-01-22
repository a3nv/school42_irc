#include "../includes/Client.hpp"
#include <cstddef>
#include <string>
#include <unistd.h>

Client::Client() : _fd(-1), _port(-1), _ip("") {
}

Client::Client(int fd, int port, const std::string& ip) : _fd(fd), _port(port), _ip(ip) {
}
Client::~Client() {
}

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

bool Client::tryRegisterClient(){
	if (!_nickname.empty() && !_username.empty() && !_name.empty()) {
		this->_registered = true;
	}
	return this->_registered;
}

int Client::getFd() const { return _fd;}
int Client::getPort() const { return _port;}
std::string Client::getIp() const { return _ip;}
std::string Client::getName() const { return _name;}
std::string Client::getNickname() const { return _nickname;}
std::string Client::getUsername() const { return _username;}
bool Client::isRegistered() const { return _registered;}
void Client::setName(const std::string& name) {_name = name;}
void Client::setNickname(const std::string& nickname) {_nickname = nickname;}
void Client::setUsername(const std::string& username) {_username = username;}
size_t Client::inbufSize() const {
	return _inbuf.size();
}
