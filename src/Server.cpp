#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Signal.hpp"
#include "../includes/Command.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <cctype>
#include <iomanip>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

// ---------------- helpers ----------------

static bool endsWithCRLF(const std::string &s) {
	if (s.size() < 2)
		return false;
	return (s[s.size() - 2] == '\r' && s[s.size() - 1] == '\n');
}

static void uppercaseCommand(std::string &s) {
	for (size_t i = 0; i < s.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(s[i]);
		s[i] = static_cast<char>(std::toupper(c));
	}
}

static void skipSpaces(const std::string &s, size_t &i) {
	while (i < s.size() && s[i] == ' ')
		++i;
}

static std::string readToken(const std::string &s, size_t &i) {
	size_t start = i;
	while (i < s.size() && s[i] != ' ')
		++i;
	return s.substr(start, i - start);
}

static std::string stripCRLF(const std::string &s) {
	if (!s.empty() && s[s.size() - 1] == '\n') {
		std::string t = s.substr(0, s.size() - 1);
		if (!t.empty() && t[t.size() - 1] == '\r')
			t.erase(t.size() - 1);
		return t;
	}
	if (!s.empty() && s[s.size() - 1] == '\r')
		return s.substr(0, s.size() - 1);
	return s;
}

// ---------------- Server ----------------

Server::Server(int port, const std::string &password)
	: _port(port), _password(password), _listenFd(-1), _clients(), _commands(), _pendingDisconnect(), _channels() {
		initCommands();
	}

Server::~Server() {
	destroyCommands();
	if (_listenFd >= 0)
		close(_listenFd);
}

const std::map<int, Client> &Server::getClients() const { return _clients; }

int Server::getClientFd(const Client &client) const { return client.getFd(); }

bool Server::setNonBlocking(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0)
		return false;
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
		return false;
	return true;
}

std::string Server::getServerName() const { return "irc42"; }
bool Server::passwordRequired() const { return !_password.empty(); }
const std::string &Server::getPassword() const { return _password; }

bool Server::isRegistered(const Client &c) const { return c.isRegistered(); }

void Server::scheduleDisconnect(int fd) {
	_pendingDisconnect.insert(fd);
}

// RFC1459 casemapping fold for nicks/channels: A-Z -> a-z, []\^ -> {}|~
std::string Server::normalizeName(const std::string &s) {
	std::string out;
	out.reserve(s.size());
	for (size_t i = 0; i < s.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(s[i]);
		if (c >= 'A' && c <= 'Z')
			out.push_back(static_cast<char>(c + 32));
		else if (c == '[') out.push_back('{');
		else if (c == ']') out.push_back('}');
		else if (c == '\\') out.push_back('|');
		else if (c == '^') out.push_back('~');
		else out.push_back(static_cast<char>(c));
	}
	return out;
}

std::string Server::normalizeChanKey(const std::string &chan) const {
	return normalizeName(chan);
}

bool Server::isNickTaken(const std::string &nickname, int exceptFd) const {
	std::string key = normalizeName(nickname);
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->first == exceptFd)
			continue;
		if (normalizeName(it->second.getNickname()) == key)
			return true;
	}
	return false;
}

int Server::findClientFdByNick(const std::string &nickname) const {
	std::string key = normalizeName(nickname);
	for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (normalizeName(it->second.getNickname()) == key)
			return it->first;
	}
	return -1;
}

std::string Server::makePrefix(const Client &c) const {
	std::string user = c.getName().empty() ? "user" : c.getName();
	std::string host = c.getIp().empty() ? "localhost" : c.getIp();
	return c.getNickname() + "!" + user + "@" + host;
}

void Server::sendToClient(int fd, const std::string &msg) {
	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return;

	std::string out = msg;
	if (!endsWithCRLF(out))
		out += "\r\n";
	it->second.appendOutput(out);
}

bool Server::flushClientOutput(int fd) {
	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return false;

	Client &c = it->second;
	while (c.hasOutput()) {
		const std::string &buf = c.outbuf();
		ssize_t n = ::send(fd, buf.c_str(), buf.size(), 0);
		if (n > 0) {
			c.consumeOutput(static_cast<size_t>(n));
			continue;
		}
		if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
			return true;
		return false;
	}
	return true;
}

void Server::sendNumeric(int fd, int code, const std::vector<std::string> &params, const std::string &trailing) {
	std::ostringstream oss;
	oss << ":" << getServerName() << " ";
	oss << std::setw(3) << std::setfill('0') << code;

	std::string nick = "*";
	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it != _clients.end() && !it->second.getNickname().empty())
		nick = it->second.getNickname();

	oss << " " << nick;
	for (size_t i = 0; i < params.size(); ++i)
		oss << " " << params[i];
	if (!trailing.empty())
		oss << " :" << trailing;

	sendToClient(fd, oss.str());
}

void Server::sendError(int fd, int code, const std::string &param) {
	if (code == ERR_NOTREGISTERED) {
		sendNumeric(fd, code, std::vector<std::string>(), "You have not registered");
		return;
	}
	if (code == ERR_NEEDMOREPARAMS) {
		std::vector<std::string> p(1, param);
		sendNumeric(fd, code, p, "Not enough parameters");
		return;
	}
	if (code == ERR_ALREADYREGISTRED) {
		sendNumeric(fd, code, std::vector<std::string>(), "You may not reregister");
		return;
	}
	if (code == ERR_PASSWDMISMATCH) {
		sendNumeric(fd, code, std::vector<std::string>(), "Password incorrect");
		return;
	}
	if (code == ERR_NOSUCHNICK) {
		std::vector<std::string> p(1, param);
		sendNumeric(fd, code, p, "No such nick");
		return;
	}
	if (code == ERR_NOSUCHCHANNEL) {
		std::vector<std::string> p(1, param);
		sendNumeric(fd, code, p, "No such channel");
		return;
	}
	if (code == ERR_CANNOTSENDTOCHAN) {
		std::vector<std::string> p(1, param);
		sendNumeric(fd, code, p, "Cannot send to channel");
		return;
	}
	if (code == ERR_NOTONCHANNEL) {
		std::vector<std::string> p(1, param);
		sendNumeric(fd, code, p, "You're not on that channel");
		return;
	}
	if (code == ERR_USERNOTINCHANNEL) {
		// param format: "<nick> <channel>"
		std::vector<std::string> p;
		std::istringstream iss(param);
		std::string nick, chan;
		iss >> nick >> chan;
		p.push_back(nick);
		p.push_back(chan);
		sendNumeric(fd, code, p, "They aren't on that channel");
		return;
	}
	if (code == ERR_NORECIPIENT) {
		sendNumeric(fd, code, std::vector<std::string>(), "No recipient given (" + param + ")");
		return;
	}
	if (code == ERR_NOTEXTTOSEND) {
		sendNumeric(fd, code, std::vector<std::string>(), "No text to send");
		return;
	}
	if (code == ERR_CHANOPRIVSNEEDED) {
		std::vector<std::string> p(1, param);
		sendNumeric(fd, code, p, "You're not channel operator");
		return;
	}
	if (code == ERR_CHANNELISFULL) {
		std::vector<std::string> p(1, param);
		sendNumeric(fd, code, p, "Cannot join channel (+l)");
		return;
	}
	if (code == ERR_INVITEONLYCHAN) {
		std::vector<std::string> p(1, param);
		sendNumeric(fd, code, p, "Cannot join channel (+i)");
		return;
	}
	if (code == ERR_BADCHANNELKEY) {
		std::vector<std::string> p(1, param);
		sendNumeric(fd, code, p, "Cannot join channel (+k)");
		return;
	}
	sendNumeric(fd, code, std::vector<std::string>(), "");
}

void Server::acceptClient(int cfd, const std::string &ip, int port) {
	setNonBlocking(cfd);
	_clients.insert(std::make_pair(cfd, Client(cfd, port, ip)));
	sendToClient(cfd, "NOTICE AUTH :*** Looking up your hostname...");
	sendToClient(cfd, "NOTICE AUTH :*** Checking ident...");
	sendToClient(cfd, "NOTICE AUTH :*** Welcome! Please register with PASS/NICK/USER");
}

bool Server::recvFromClient(int fd) {
	char buf[4096];
	ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
	if (n == 0)
		return false;
	if (n < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return true;
		return false;
	}

	std::map<int, Client>::iterator it = _clients.find(fd);
	if (it == _clients.end())
		return false;

	it->second.appendInput(buf, static_cast<size_t>(n));

	std::string line;
	while (it->second.extractLine(line)) {
		handleLine(fd, line);
		if (_pendingDisconnect.count(fd))
			break;
	}
	return true;
}


void Server::cleanup() {
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		close(it->first);
	_clients.clear();
	_channels.clear();
	_pendingDisconnect.clear();
}

void Server::run() {
	sockaddr_in addr;
	int opt;

	_listenFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenFd < 0)
		throw std::runtime_error("socket() failed");

	opt = 1;
	if (setsockopt(_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");

	if (!setNonBlocking(_listenFd))
		throw std::runtime_error("fcntl(O_NONBLOCK) failed on listen socket");

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(static_cast<unsigned short>(_port));

	if (bind(_listenFd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0)
		throw std::runtime_error("bind() failed");

	if (listen(_listenFd, 10) < 0)
		throw std::runtime_error("listen() failed");

	std::cout << "Listening on port " << _port << std::endl;

	fd_set readfds;
	fd_set writefds;
	int maxfd;

	while (g_running) {
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);

		FD_SET(_listenFd, &readfds);
		maxfd = _listenFd;

		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			int fd = it->first;
			FD_SET(fd, &readfds);
			if (it->second.hasOutput())
				FD_SET(fd, &writefds);
			if (fd > maxfd)
				maxfd = fd;
		}

		int activity = select(maxfd + 1, &readfds, &writefds, NULL, NULL);
		if (activity < 0) {
			if (errno == EINTR)
				continue;
			perror("select");
			break;
		}

		if (FD_ISSET(_listenFd, &readfds)) {
			sockaddr_in client;
			socklen_t len = sizeof(client);
			int cfd = accept(_listenFd, reinterpret_cast<sockaddr *>(&client), &len);
			if (cfd >= 0) {
				std::string ip = inet_ntoa(client.sin_addr);
				int port = ntohs(client.sin_port);
				acceptClient(cfd, ip, port);
			}
		}

		for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end();) {
			int fd = it->first;

			bool drop = false;

			if (FD_ISSET(fd, &readfds)) {
				if (!recvFromClient(fd))
					drop = true;
			}
			if (!drop && FD_ISSET(fd, &writefds)) {
				if (!flushClientOutput(fd))
					drop = true;
			}
			if (!drop && _pendingDisconnect.count(fd))
				drop = true;

			if (drop) {
				std::string quitMsg = "Client quit";
				removeClientFromAllChannels(fd, quitMsg);
				close(fd);
				_pendingDisconnect.erase(fd);
				_clients.erase(it++);
				continue;
			}
			++it;
		}
	}

	std::cout << "Server is shutting down." << std::endl;
}

// ---------------- Parsing / dispatch ----------------

void Server::handleLine(int fd, const std::string &rawLine) {
	std::string line = stripCRLF(rawLine);
	if (line.empty())
		return;

	std::map<int, Client>::iterator cit = _clients.find(fd);
	if (cit == _clients.end())
		return;

	IrcMessage msg;
	size_t i = 0;

	// prefix
	if (i < line.size() && line[i] == ':') {
		++i;
		size_t start = i;
		while (i < line.size() && line[i] != ' ')
			++i;
		msg.prefix = line.substr(start, i - start);
		skipSpaces(line, i);
	}

	// command
	msg.command = readToken(line, i);
	uppercaseCommand(msg.command);
	skipSpaces(line, i);

	// params
	while (i < line.size()) {
		if (line[i] == ':') {
			++i;
			msg.params.push_back(line.substr(i));
			break;
		}
		msg.params.push_back(readToken(line, i));
		skipSpaces(line, i);
	}

	dispatchCommand(fd, cit->second, msg);
}

void Server::dispatchCommand(int fd, Client &client, const IrcMessage &msg) {
	std::map<std::string, Command *>::iterator it = _commands.find(msg.command);
	if (it == _commands.end()) {
		sendError(fd, ERR_UNKNOWNCOMMAND, msg.command);
		return;
	}
	it->second->run(*this, fd, client, msg);
}

// ---------------- registration ----------------

void Server::tryRegister(int fd, Client &client) {
	if (client.isRegistered())
		return;
	if (passwordRequired() && !client.hasPass())
		return;
	if (!client.hasNick() || !client.hasUser())
		return;

	client.setRegistered(true);
	sendNumeric(fd, RPL_WELCOME, std::vector<std::string>(),
	            "Welcome to the Internet Relay Network " + makePrefix(client));
	sendNumeric(fd, RPL_YOURHOST, std::vector<std::string>(),
	            "Your host is " + getServerName() + ", running version 0.1");
	sendNumeric(fd, RPL_CREATED, std::vector<std::string>(), "This server was created today");
	sendNumeric(fd, RPL_MYINFO, std::vector<std::string>(), getServerName() + " 0.1");
}

// ---------------- channels core ----------------

bool Server::isChannelName(const std::string &target) const {
	return !target.empty() && target[0] == '#';
}

Channel *Server::findChannelByKey(const std::string &key) {
	std::map<std::string, Channel>::iterator it = _channels.find(key);
	if (it == _channels.end())
		return NULL;
	return &it->second;
}

Channel &Server::getOrCreateChannel(const std::string &displayName) {
	std::string key = normalizeChanKey(displayName);
	std::map<std::string, Channel>::iterator it = _channels.find(key);
	if (it == _channels.end()) {
		_channels.insert(std::make_pair(key, Channel(displayName, key)));
		it = _channels.find(key);
	}
	return it->second;
}

void Server::broadcastToChannel(Channel &ch, const std::string &line, int exceptFd) {
	for (std::set<int>::const_iterator it = ch.members().begin(); it != ch.members().end(); ++it) {
		int fd = *it;
		if (fd == exceptFd)
			continue;
		sendToClient(fd, line);
	}
}

void Server::removeClientFromAllChannels(int fd, const std::string &quitMsg) {
	std::map<int, Client>::iterator cit = _clients.find(fd);
	if (cit == _clients.end())
		return;

	Client &c = cit->second;
	std::set<std::string> keys = c.getChannelKeys();

	for (std::set<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it) {
		std::map<std::string, Channel>::iterator chIt = _channels.find(*it);
		if (chIt == _channels.end())
			continue;

		Channel &ch = chIt->second;
		std::string line = ":" + makePrefix(c) + " QUIT :" + quitMsg;
		broadcastToChannel(ch, line, fd);

		ch.removeMember(fd);
		c.partChannelKey(*it);

		if (ch.empty())
			_channels.erase(chIt);
	}
}

void Server::joinChannel(int fd, Client &client, const std::string &chanName, const std::string &keyArg) {
	if (!isChannelName(chanName)) {
		sendError(fd, ERR_NOSUCHCHANNEL, chanName);
		return;
	}

	Channel &ch = getOrCreateChannel(chanName);
	std::string key = ch.key();

	if (client.isInChannelKey(key))
		return;

	if (ch.inviteOnly() && !ch.isInvited(fd)) {
		sendError(fd, ERR_INVITEONLYCHAN, ch.name());
		return;
	}
	if (ch.hasKey() && ch.keyPassword() != keyArg) {
		sendError(fd, ERR_BADCHANNELKEY, ch.name());
		return;
	}
	if (ch.hasLimit() && ch.members().size() >= ch.limit()) {
		sendError(fd, ERR_CHANNELISFULL, ch.name());
		return;
	}

	ch.addMember(fd);
	client.joinChannelKey(key);
	ch.uninvite(fd);

	if (ch.ops().empty())
		ch.addOp(fd);

	std::string joinLine = ":" + makePrefix(client) + " JOIN :" + ch.name();
	broadcastToChannel(ch, joinLine, -1);

	if (ch.hasTopic()) {
		std::vector<std::string> p;
		p.push_back(ch.name());
		sendNumeric(fd, RPL_TOPIC, p, ch.topic());
	} else {
		std::vector<std::string> p;
		p.push_back(ch.name());
		sendNumeric(fd, RPL_NOTOPIC, p, "No topic is set");
	}
	sendNames(fd, client, ch.name());
}

void Server::partChannel(int fd, Client &client, const std::string &chanName, const std::string &partMsg) {
	std::string key = normalizeChanKey(chanName);
	Channel *chp = findChannelByKey(key);
	if (!chp) {
		sendError(fd, ERR_NOSUCHCHANNEL, chanName);
		return;
	}
	Channel &ch = *chp;
	if (!ch.hasMember(fd)) {
		sendError(fd, ERR_NOTONCHANNEL, ch.name());
		return;
	}

	std::string msg = partMsg.empty() ? "Leaving" : partMsg;
	std::string line = ":" + makePrefix(client) + " PART " + ch.name() + " :" + msg;
	broadcastToChannel(ch, line, -1);

	ch.removeMember(fd);
	client.partChannelKey(key);

	if (ch.empty())
		_channels.erase(key);
}

void Server::sendNames(int fd, Client &client, const std::string &chanName) {
	(void) client;
	std::string key = normalizeChanKey(chanName);
	Channel *chp = findChannelByKey(key);
	if (!chp) {
		std::vector<std::string> p;
		p.push_back(chanName);
		sendNumeric(fd, RPL_ENDOFNAMES, p, "End of /NAMES list.");
		return;
	}

	Channel &ch = *chp;
	std::ostringstream names;
	bool first = true;

	for (std::set<int>::const_iterator it = ch.members().begin(); it != ch.members().end(); ++it) {
		std::map<int, Client>::iterator cit = _clients.find(*it);
		if (cit == _clients.end())
			continue;
		std::string nick = cit->second.getNickname();
		if (nick.empty())
			continue;

		if (!first)
			names << " ";
		first = false;

		if (ch.isOp(*it))
			names << "@" << nick;
		else
			names << nick;
	}

	std::vector<std::string> p;
	p.push_back("=");
	p.push_back(ch.name());
	sendNumeric(fd, RPL_NAMREPLY, p, names.str());

	std::vector<std::string> endp;
	endp.push_back(ch.name());
	sendNumeric(fd, RPL_ENDOFNAMES, endp, "End of /NAMES list.");
}

void Server::channelPrivmsg(int fd, Client &client, const std::string &chanName, const std::string &text) {
	std::string key = normalizeChanKey(chanName);
	Channel *chp = findChannelByKey(key);
	if (!chp) {
		sendError(fd, ERR_NOSUCHCHANNEL, chanName);
		return;
	}
	Channel &ch = *chp;
	if (!ch.hasMember(fd)) {
		sendError(fd, ERR_CANNOTSENDTOCHAN, ch.name());
		return;
	}

	std::string line = ":" + makePrefix(client) + " PRIVMSG " + ch.name() + " :" + text;
	broadcastToChannel(ch, line, fd);
}

void Server::getChannelTopic(int fd, Client &client, const std::string &chanName) {
	(void) client;
	std::string key = normalizeChanKey(chanName);
	Channel *chp = findChannelByKey(key);
	if (!chp) {
		sendError(fd, ERR_NOSUCHCHANNEL, chanName);
		return;
	}
	Channel &ch = *chp;

	if (ch.hasTopic()) {
		std::vector<std::string> p;
		p.push_back(ch.name());
		sendNumeric(fd, RPL_TOPIC, p, ch.topic());
	} else {
		std::vector<std::string> p;
		p.push_back(ch.name());
		sendNumeric(fd, RPL_NOTOPIC, p, "No topic is set");
	}
}

void Server::setChannelTopic(int fd, Client &client, const std::string &chanName, const std::string &topic) {
	std::string key = normalizeChanKey(chanName);
	Channel *chp = findChannelByKey(key);
	if (!chp) {
		sendError(fd, ERR_NOSUCHCHANNEL, chanName);
		return;
	}
	Channel &ch = *chp;

	if (!ch.hasMember(fd)) {
		sendError(fd, ERR_NOTONCHANNEL, ch.name());
		return;
	}
	if (ch.topicOpOnly() && !ch.isOp(fd)) {
		sendError(fd, ERR_CHANOPRIVSNEEDED, ch.name());
		return;
	}

	ch.setTopic(topic);
	std::string line = ":" + makePrefix(client) + " TOPIC " + ch.name() + " :" + topic;
	broadcastToChannel(ch, line, -1);
}

static bool parseSizeT(const std::string &s, size_t &out) {
	if (s.empty())
		return false;
	size_t v = 0;
	for (size_t i = 0; i < s.size(); ++i) {
		if (s[i] < '0' || s[i] > '9')
			return false;
		v = v * 10 + static_cast<size_t>(s[i] - '0');
	}
	out = v;
	return true;
}

void Server::channelMode(int fd, Client &client, const IrcMessage &msg) {
	if (msg.params.empty()) {
		sendError(fd, ERR_NEEDMOREPARAMS, "MODE");
		return;
	}
	std::string chanName = msg.params[0];
	if (!isChannelName(chanName))
		return;

	std::string key = normalizeChanKey(chanName);
	Channel *chp = findChannelByKey(key);
	if (!chp) {
		sendError(fd, ERR_NOSUCHCHANNEL, chanName);
		return;
	}
	Channel &ch = *chp;

	if (msg.params.size() == 1) {
		std::vector<std::string> p;
		p.push_back(ch.name());
		p.push_back(ch.modeString());
		sendNumeric(fd, RPL_CHANNELMODEIS, p, "");
		return;
	}

	if (!ch.isOp(fd)) {
		sendError(fd, ERR_CHANOPRIVSNEEDED, ch.name());
		return;
	}

	std::string modes = msg.params[1];
	int sign = 0;
	size_t argi = 2;

	for (size_t i = 0; i < modes.size(); ++i) {
		char m = modes[i];
		if (m == '+') {
			sign = +1;
			continue;
		}
		if (m == '-') {
			sign = -1;
			continue;
		}
		if (sign == 0) continue;

		if (m == 'i') {
			ch.setInviteOnly(sign == +1);
		} else if (m == 't') {
			ch.setTopicOpOnly(sign == +1);
		} else if (m == 'k') {
			if (sign == +1) {
				if (argi >= msg.params.size())
					continue;
				ch.setKeyPassword(msg.params[argi++]);
			} else {
				ch.clearKeyPassword();
			}
		} else if (m == 'l') {
			if (sign == +1) {
				if (argi >= msg.params.size())
					continue;
				size_t lim;
				if (parseSizeT(msg.params[argi++], lim))
					ch.setLimit(lim);
			} else {
				ch.clearLimit();
			}
		} else if (m == 'o') {
			if (argi >= msg.params.size())
				continue;
			std::string nick = msg.params[argi++];
			int tfd = findClientFdByNick(nick);
			if (tfd < 0) {
				sendError(fd, ERR_NOSUCHNICK, nick);
				continue;
			}
			if (!ch.hasMember(tfd)) {
				sendError(fd, ERR_USERNOTINCHANNEL, nick + " " + ch.name());
				continue;
			}
			if (sign == +1)
				ch.addOp(tfd);
			else
				ch.removeOp(tfd);
		}
	}

	std::ostringstream line;
	line << ":" << makePrefix(client) << " MODE " << ch.name() << " " << msg.params[1];
	for (size_t k = 2; k < msg.params.size(); ++k)
		line << " " << msg.params[k];
	broadcastToChannel(ch, line.str(), -1);
}

void Server::channelInvite(int fd, Client &client, const std::string &nick, const std::string &chanName) {
	std::string key = normalizeChanKey(chanName);
	Channel *chp = findChannelByKey(key);
	if (!chp) {
		sendError(fd, ERR_NOSUCHCHANNEL, chanName);
		return;
	}
	Channel &ch = *chp;
	if (!ch.isOp(fd)) {
		sendError(fd, ERR_CHANOPRIVSNEEDED, ch.name());
		return;
	}
	int tfd = findClientFdByNick(nick);
	if (tfd < 0) {
		sendError(fd, ERR_NOSUCHNICK, nick);
		return;
	}
	ch.invite(tfd);

	std::vector<std::string> p;
	p.push_back(nick);
	p.push_back(ch.name());
	sendNumeric(fd, RPL_INVITING, p, "");

	std::map<int, Client>::iterator tit = _clients.find(tfd);
	if (tit != _clients.end()) {
		sendToClient(tfd, ":" + makePrefix(client) + " INVITE " + nick + " :" + ch.name());
	}
}

void Server::channelKick(int fd, Client &client, const std::string &chanName, const std::string &nick,
                         const std::string &reason) {
	std::string key = normalizeChanKey(chanName);
	Channel *chp = findChannelByKey(key);
	if (!chp) {
		sendError(fd, ERR_NOSUCHCHANNEL, chanName);
		return;
	}
	Channel &ch = *chp;
	if (!ch.isOp(fd)) {
		sendError(fd, ERR_CHANOPRIVSNEEDED, ch.name());
		return;
	}
	int tfd = findClientFdByNick(nick);
	if (tfd < 0) {
		sendError(fd, ERR_NOSUCHNICK, nick);
		return;
	}
	if (!ch.hasMember(tfd)) {
		sendError(fd, ERR_USERNOTINCHANNEL, nick + " " + ch.name());
		return;
	}

	std::string msg = reason.empty() ? nick : reason;
	std::string line = ":" + makePrefix(client) + " KICK " + ch.name() + " " + nick + " :" + msg;
	broadcastToChannel(ch, line, -1);

	std::map<int, Client>::iterator tit = _clients.find(tfd);
	if (tit != _clients.end()) {
		tit->second.partChannelKey(key);
	}
	ch.removeMember(tfd);

	if (ch.empty())
		_channels.erase(key);
}

// ---------------- commands registry ----------------

void Server::initCommands() {
	_commands["CAP"] = new Cap();
	_commands["PASS"] = new Pass();
	_commands["NICK"] = new Nick();
	_commands["USER"] = new User();
	_commands["PING"] = new Ping();
	_commands["PONG"] = new Pong();
	_commands["PRIVMSG"] = new PrivMsg();
	_commands["NOTICE"] = new Notice();
	_commands["QUIT"] = new Quit();

	_commands["JOIN"] = new Join();
	_commands["PART"] = new Part();
	_commands["NAMES"] = new Names();

	_commands["MODE"] = new Mode();
	_commands["TOPIC"] = new Topic();
	_commands["INVITE"] = new Invite();
	_commands["KICK"] = new Kick();
}

void Server::destroyCommands() {
	for (std::map<std::string, Command *>::iterator it = _commands.begin(); it != _commands.end(); ++it)
		delete it->second;
	_commands.clear();
}
