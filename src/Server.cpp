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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <algorithm> // for std::remove - consider we probably are not allowed to use this

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
					close(fd);
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
	}
	return true;
}

// Accept a new client and add to the clients map
void Server::acceptClient(int cfd, const std::string& ip, int port) {
	this->_clients.insert(std::make_pair(cfd, Client(cfd, port, ip)));
}

void Server::cleanup() {
	for (std::map<int, Client>::iterator it = _clients.begin();
			it != _clients.end(); ++it) {
		close(it->first);
	}
	_clients.clear();
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
    std::map<std::string, Command*>::iterator it;
	std::cout << "fd " << fd << " LINE: [" << line << "]" << std::endl;
	if (!parse(line, msg)) {
        std::cout << "Parse: empty/invalid line\n";
        return;
    }
	cit = _clients.find(fd);
    if (cit == _clients.end())
        return;

    it = _commands.find(msg.command);
    if (it == _commands.end()) {
        sendToClient(fd, ":irc42 421 " + msg.command + " :Unknown command");
        return;
    }
    it->second->run(*this, fd, cit->second, msg);
}

static bool endsWithCRLF(const std::string &s)
{
    if (s.size() < 2)
        return false;
    return (s[s.size() - 2] == '\r' && s[s.size() - 1] == '\n');
}
void Server::sendToClient(int fd, const std::string &msg)
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
    _commands["PING"] = new Ping();
}

void Server::destroyCommands()
{
    std::map<std::string, Command*>::iterator it;

    it = _commands.begin();
    while (it != _commands.end()) {
        delete it->second;
        ++it;
    }
    _commands.clear();
}
