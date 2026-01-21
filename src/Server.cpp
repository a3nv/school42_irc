#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Signal.hpp"
#include "../includes/Command.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <cctype>
#include <iomanip>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

static bool endsWithCRLF(const std::string &s)
{
    if (s.size() < 2)
        return false;
    return (s[s.size() - 2] == '\r' && s[s.size() - 1] == '\n');
}

static std::string numeric3(int code)
{
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << code;
    return oss.str();
}

static std::string serverName()
{
    return "irc42";
}

Server::Server(int port, const std::string& password)
	: _port(port), _password(password), _listenFd(-1) {
		initCommands();
	}

Server::~Server() {
	destroyCommands();
	cleanup();
}

const std::map<int, Client>& Server::getClients() const {
	return _clients;
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

	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(static_cast<unsigned short>(_port));

	if (bind(_listenFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
		throw std::runtime_error("bind() failed");

	if (listen(_listenFd, 10) < 0)
		throw std::runtime_error("listen() failed");

	std::cout << "Listening on port " << _port << std::endl;

	fd_set readfds;
	int maxfd;

	while (g_running) {
		FD_ZERO(&readfds);

		//Add listening socket
		FD_SET(_listenFd, &readfds);
		maxfd = _listenFd;

		//Add all client sockets
		for (std::map<int, Client>::iterator it = _clients.begin();
				it != _clients.end();
				++it)
		{
			int fd = it->first;
			std::cout << "Checking fd: " << fd << std::endl;
			std::cout << "Current maxfd: " << maxfd << std::endl;
			FD_SET(fd, &readfds);
			if (fd > maxfd)
				maxfd = fd;
		}

		//Call select
		int activity = select(maxfd + 1, &readfds, NULL, NULL, NULL);

		if (activity < 0) {
			if (errno == EINTR)
				continue;
			perror("select");
			break;
		}

		// Check listening socket
		if (FD_ISSET(_listenFd, &readfds)) {
			sockaddr_in client;
			socklen_t len = sizeof(client);
			int cfd = accept(_listenFd, reinterpret_cast<sockaddr*>(&client), &len);
			if (cfd < 0) {
				perror("accept");
				continue;
			}

			std::string ip = inet_ntoa(client.sin_addr);//IP address
			int port = ntohs(client.sin_port);          //Port number
			acceptClient(cfd, ip, port);               //Accept the client

			std::cout << "Client connected from "
				<< ip
				<< ":" << port
				<< " (fd: " << cfd << ")"
				<< std::endl;
			std::cout << "Total clients: " << _clients.size() << std::endl;
		}

		//Check client sockets
		for (std::map<int, Client>::iterator it = _clients.begin();
				it != _clients.end(); )
		{
			int fd = it->first;

			if (FD_ISSET(fd, &readfds)) {
				if (!recvFromClient(fd)) {
					removeFromAllChannels(fd);
					close(fd);
					_pendingDisconnect.erase(fd);
					_clients.erase(it++);
					continue;
				}
				if (_pendingDisconnect.count(fd)) {
					removeFromAllChannels(fd);
					close(fd);
					_pendingDisconnect.erase(fd);
					_clients.erase(it++);
					continue;
				}
			}
			++it;
		}
	}

	std::cout << "Server is shutting down." << std::endl;
}

bool Server::recvFromClient(int fd) {
	char buffer[512];
	int bytesRead;
	std::map<int, Client>::iterator it;
	std::string line;
	
	it = _clients.find(fd);
	if (it == _clients.end())
		return false;

	bytesRead = recv(fd, buffer, sizeof(buffer), 0);
	if (bytesRead <= 0) {
		if (bytesRead < 0)
			perror("recv");
		std::cout << "Client disconnected. fd: " << fd << std::endl;
		return false;
	}
	it->second.appendInput(buffer, (size_t) bytesRead);
	if (it->second.inbufSize() > 8192) {
		std::cout << "Client input buffer too large. fd: " << fd << std::endl;
		return false;	
	}
	while (it->second.extractLine(line)) {
		handleLine(fd, line);
		if (_pendingDisconnect.count(fd))
			break;
	}
	return true;
}

// Accept a new client and add to the clients map
void Server::acceptClient(int cfd, const std::string& ip, int port) {
	this->_clients.insert(std::make_pair(cfd, Client(cfd, port, ip)));
	sendToClient(cfd, "NOTICE AUTH :*** Looking up your hostname...");
	sendToClient(cfd, "NOTICE AUTH :*** Checking ident...");
	sendToClient(cfd, "NOTICE AUTH :*** Welcome! Please register with PASS/NICK/USER");
}

void Server::cleanup() {
	for (std::map<int, Client>::iterator it = _clients.begin();
			it != _clients.end(); ++it) {
		close(it->first);
	}
	_clients.clear();
	_channels.clear();
	if (_listenFd != -1) {
		close(_listenFd);
		_listenFd = -1;
	}
}

static void normalizeCommand(std::string &s) {
	for (size_t i = 0; i < s.size(); ++i) {
		unsigned char c = static_cast<unsigned char>(s[i]);
		s[i] = static_cast<char>(std::toupper(c));
	}
}

static void skipSpaces(const std::string &s, size_t &i) {
	while(i < s.size() && s[i] == ' ')
		++i;
}

static std::string readToken(const std::string &s, size_t &i) {
    size_t start = i;
    while (i < s.size() && s[i] != ' ')
        ++i;
    return s.substr(start, i - start);
}

static std::string stripCRLF(const std::string &line) {
    size_t end = line.size();
    while (end > 0 && (line[end - 1] == '\r' || line[end - 1] == '\n'))
        --end;
    return line.substr(0, end);
}

static bool parse(const std::string &raw, IrcMessage &out) {
	std::string line = stripCRLF(raw);
	size_t i = 0;
	out.prefix.clear();
	out.command.clear();
	out.params.clear();
	skipSpaces(line, i);
	if (i >= line.size())
		return false;
	if (line[i] == ':') {
		++i;
		if (i >= line.size())
			return false;
		out.prefix = readToken(line, i);
		skipSpaces(line, i);
		if (i >= line.size())
			return false;
	}
	out.command = readToken(line, i);
    if (out.command.empty())
        return false;
    normalizeCommand(out.command);
	while (i < line.size()) {
        skipSpaces(line, i);
        if (i >= line.size())
            break;

        if (line[i] == ':') {
            out.params.push_back(line.substr(i + 1));
            break;
        } else {
            out.params.push_back(readToken(line, i));
        }
    }
    return true;
}

void Server::handleLine(int fd, const std::string &line) {
	IrcMessage msg;
	std::map<int, Client>::iterator cit;
	std::cout << "fd " << fd << " LINE: [" << line << "]" << std::endl;
	if (!parse(line, msg)) {
		std::cout << "Parse: empty/invalid line\n";
		return;
	}
	cit = _clients.find(fd);
	if (cit == _clients.end())
		return;
	for (size_t k = 0; k < msg.params.size(); ++k)
		std::cout << "ARG[" << k << "]: " << msg.params[k] << "\n";
	dispatchCommand(fd, cit->second, msg);
}

void Server::sendToClient(int fd, const std::string &msg) const
{
    std::string out;

    out = msg;
    if (!endsWithCRLF(out))
        out += "\r\n";
    if (::send(fd, out.c_str(), out.size(), MSG_NOSIGNAL) < 0)
        perror("send");
}

void Server::initCommands()
{
	_commands["PASS"] = new Pass();
	_commands["NICK"] = new Nick();
	_commands["USER"] = new User();
	_commands["PING"] = new Ping();
	_commands["PONG"] = new Pong();
	_commands["PRIVMSG"] = new PrivMsg();
	_commands["NOTICE"] = new Notice();
	_commands["JOIN"] = new Join();
	_commands["PART"] = new Part();
	_commands["NAMES"] = new Names();
	_commands["QUIT"] = new Quit();
}

void Server::destroyCommands()
{
    std::map<std::string, Command*>::iterator it;

	for (it = _commands.begin(); it != _commands.end(); ++it)
        delete it->second;
    _commands.clear();
}

void Server::dispatchCommand(int fd, Client &client, const IrcMessage &msg) {
	std::map<std::string, Command*>::iterator it;
	it = _commands.find(msg.command);
	if (it == _commands.end()) {
		sendError(fd, ERR_UNKNOWNCOMMAND, msg.command);
		return;
	}
	it->second->run(*this, fd, client, msg);
}

void Server::sendError(int fd, int code, const std::string &param) const {
	std::map<int, Client>::const_iterator it;
    std::string target;
    std::string line;

    target = "*";
    it = _clients.find(fd);
    if (it != _clients.end() && !it->second.getNickname().empty())
        target = it->second.getNickname();

    line = ":" + serverName() + " " + numeric3(code) + " " + target;

    switch (code) {
        case ERR_UNKNOWNCOMMAND:
            line += " " + param + " :Unknown command";
            break;
        case ERR_NONICKNAMEGIVEN:
            line += " :No nickname given";
            break;
        case ERR_ERRONEUSNICKNAME:
            line += " " + param + " :Erroneous nickname";
            break;
        case ERR_NICKNAMEINUSE:
            line += " " + param + " :Nickname is already in use";
            break;
		case ERR_NOTREGISTERED:
			line += " :You have not registered";
			break;
		case ERR_NEEDMOREPARAMS:
			line += " " + param + " :Not enough parameters";
			break;
		case ERR_ALREADYREGISTRED:
			line += " :You may not reregister";
			break;
		case ERR_PASSWDMISMATCH:
			line += " :Password incorrect";
			break;
		case ERR_NOSUCHNICK:
			line += " " + param + " :No such nick";
			break;
		case ERR_NORECIPIENT:
			line += " :No recipient given (" + param + ")";
			break;
		case ERR_NOTEXTTOSEND:
			line += " :No text to send";
			break;
		case ERR_NOSUCHCHANNEL:
			line += " " + param + " :No such channel";
			break;
		case ERR_CANNOTSENDTOCHAN:
			line += " " + param + " :Cannot send to channel";
			break;
		case ERR_NOTONCHANNEL:
			line += " " + param + " :You're not on that channel";
			break;
        default:
            line += " :Unknown error";
            break;
    }
    sendToClient(fd, line);
}

bool Server::isNickTaken(const std::string &nickname, int exceptFd) const
{
    for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->first != exceptFd && it->second.getNickname() == nickname)
            return true;
    }
    return false;
}

std::string Server::getServerName() const { 
	return "irc42"; 
}

bool Server::passwordRequired() const { 
	return !_password.empty(); 
}
const std::string &Server::getPassword() const { 
	return _password; 
}

void Server::scheduleDisconnect(int fd) { 
	_pendingDisconnect.insert(fd); 
}

int Server::findClientFdByNick(const std::string &nickname) const {
    for (std::map<int, Client>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.getNickname() == nickname)
            return it->first;
    }
    return -1;
}

bool Server::isRegistered(const Client &c) const { return c.isRegistered(); }

void Server::tryRegister(int fd, Client &client)
{
    std::string nick;
    std::string line;

    if (client.isRegistered())
        return;
    if (passwordRequired() && !client.hasPass())
        return;
    if (!client.hasNick() || !client.hasUser())
        return;

    client.setRegistered(true);

    nick = client.getNickname();
    sendToClient(fd, ":" + getServerName() + " 001 " + nick + " :Welcome to the IRC server " + nick);
    sendToClient(fd, ":" + getServerName() + " 002 " + nick + " :Your host is " + getServerName());
    sendToClient(fd, ":" + getServerName() + " 003 " + nick + " :This server was created just now");
    sendToClient(fd, ":" + getServerName() + " 004 " + nick + " " + getServerName() + " 1.0 o o");
}

static char foldRfc1459Char(char c)
{
    unsigned char uc;

    uc = static_cast<unsigned char>(c);
    if (uc >= 'A' && uc <= 'Z')
        return static_cast<char>(uc - 'A' + 'a');
    if (c == '[')
        return '{';
    if (c == ']')
        return '}';
    if (c == '\\')
        return '|';
    if (c == '^')
        return '~';
    return c;
}

std::string Server::normalizeChanKey(const std::string &name)
{
    std::string out;
    size_t i;

    out.reserve(name.size());
    i = 0;
    while (i < name.size()) {
        out.push_back(foldRfc1459Char(name[i]));
        ++i;
    }
    return out;
}

bool Server::channelExists(const std::string &key) const
{
    return _channels.find(key) != _channels.end();
}

bool Server::isClientInChannel(const std::string &key, int fd) const
{
    std::map<std::string, Channel>::const_iterator it;

    it = _channels.find(key);
    if (it == _channels.end())
        return false;
    return it->second.hasMember(fd);
}

void Server::joinChannel(const std::string &key, const std::string &displayName, int fd)
{
    std::map<std::string, Channel>::iterator it;

    it = _channels.find(key);
    if (it == _channels.end())
        it = _channels.insert(std::make_pair(key, Channel(displayName))).first;
    it->second.addMember(fd);
}

void Server::partChannel(const std::string &key, int fd)
{
    std::map<std::string, Channel>::iterator it;

    it = _channels.find(key);
    if (it == _channels.end())
        return;
    it->second.removeMember(fd);
    if (it->second.empty())
        _channels.erase(it);
}

void Server::removeFromAllChannels(int fd)
{
    std::map<int, Client>::iterator cit;
    std::vector<std::string> keys;
    std::set<std::string>::const_iterator it;

    cit = _clients.find(fd);
    if (cit == _clients.end())
        return;
    for (it = cit->second.getChannels().begin(); it != cit->second.getChannels().end(); ++it)
        keys.push_back(*it);
    for (size_t i = 0; i < keys.size(); ++i) {
        partChannel(keys[i], fd);
        cit->second.partChannel(keys[i]);
    }
}

void Server::sendToChannel(const std::string &key, const std::string &line, int exceptFd) const
{
    std::map<std::string, Channel>::const_iterator it;
    std::set<int>::const_iterator mit;

    it = _channels.find(key);
    if (it == _channels.end())
        return;
    for (mit = it->second.members().begin(); mit != it->second.members().end(); ++mit) {
        if (*mit == exceptFd)
            continue;
        sendToClient(*mit, line);
    }
}

std::string Server::getChannelDisplayName(const std::string &key) const
{
    std::map<std::string, Channel>::const_iterator it;

    it = _channels.find(key);
    if (it == _channels.end())
        return key;
    return it->second.name();
}

std::string Server::getChannelDisplayNameOrRaw(const std::string &key, const std::string &raw) const
{
    std::map<std::string, Channel>::const_iterator it;

    it = _channels.find(key);
    if (it == _channels.end())
        return raw;
    return it->second.name();
}

void Server::sendNamesReply(int fd, const Client &requester, const std::string &key) const
{
    std::map<std::string, Channel>::const_iterator it;
    std::set<int>::const_iterator mit;
    std::string nick;
    std::string chan;
    std::string names;

    it = _channels.find(key);
    if (it == _channels.end())
        return;

    nick = requester.getNickname();
    if (nick.empty())
        nick = "*";
    chan = it->second.name();

    for (mit = it->second.members().begin(); mit != it->second.members().end(); ++mit) {
        std::map<int, Client>::const_iterator cit = _clients.find(*mit);
        if (cit == _clients.end())
            continue;
        if (!names.empty())
            names += " ";
        names += cit->second.getNickname();
    }

    sendToClient(fd, ":" + getServerName() + " " + numeric3(RPL_NAMREPLY) + " " + nick + " = " + chan + " :" + names);
    sendToClient(fd, ":" + getServerName() + " " + numeric3(RPL_ENDOFNAMES) + " " + nick + " " + chan + " :End of /NAMES list.");
}