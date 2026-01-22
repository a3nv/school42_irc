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
	}

Server::~Server() {
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
	std::cout << "fd " << fd << " LINE: [" << line << "]" << std::endl;
	if (!parse(line, msg)) {
        std::cout << "Parse: empty/invalid line\n";
        return;
    }
	std::cout << "CMD: " << msg.command << "\n";
    if (!msg.prefix.empty())
        std::cout << "PREFIX: " << msg.prefix << "\n";

    for (size_t k = 0; k < msg.params.size(); ++k){
		std::cout << "ARG[" << k << "]: " << msg.params[k] << "\n";
	}
	
	dispatchCommand(fd, msg);
}

void Server::dispatchCommand(int fd, const IrcMessage &msg) {
	if (msg.command == "NICK") {
		handleNick(fd, msg);
	} else if (msg.command == "USER") {
		// Handle USER command
	} else if (msg.command == "PING") {
		// Handle PING command
	} else {
		sendError(fd, 421, msg.params.empty() ? "" : msg.params[0]); // 421 = Unknown command
	}
}
// --------------Handle NICK command-----------------

void Server::handleNick(int fd, const IrcMessage &msg) {
	if (msg.params.size() < 1) {
		sendError(fd, ERR_NONICKNAMEGIVEN, ""); // 431 = No nickname given
		return;
	}
	Nick newNickCommand;
	std::string newNick = msg.params[0];
	newNickCommand.execute(*this, const_cast<IrcMessage&>(msg), fd);

	_clients[fd].setNickname(newNick);
	if (_clients[fd].isRegistered() == false) {
		_clients[fd].tryRegisterClient();
		if (_clients[fd].isRegistered()) {
			// Send welcome message
			std::cout << "Client fd " << fd << " registered successfully." << std::endl;
		}
	}
	std::cout << "Client fd " << fd << " set nickname to " << newNick << std::endl;
	
}
bool Server::uniqueNickname(const std::string& nickname, int fd) const{
	for (std::map<int, Client>::const_iterator it = _clients.begin();
			it != _clients.end(); ++it) {
		if (it->second.getNickname() == nickname && !(it->first == fd)) {
			return false; // Nickname already in use
		}
	}
	return true;
}

// --------------Handle USER command-----------------

void Server::handleUser(int fd, const IrcMessage &msg) {
	if (_clients[fd].isRegistered()) {
		sendError(fd, ERR_USERISREGISTERED, "");
		return;
	}
	User newUserCommand;
	newUserCommand.execute(*this, const_cast<IrcMessage&>(msg), fd);
}

// --------------Error Handling-----------------
void Server::sendError(int fd, int errorCode, std::string param) const{
	std::string errorMsg;
	switch (errorCode) {
		case ERR_UNKNOWNCOMMAND:
			errorMsg = "421 " + param + " :Unknown command";
			break;
		case ERR_NONICKNAMEGIVEN:
			errorMsg = "431 :No nickname given";
			break;
		case ERR_ERRONEUSNICKNAME:
			errorMsg = "432 " + param + " :Erroneous nickname";
			break;
		case ERR_NICKNAMEINUSE:
			errorMsg = "433 " + param + " :Nickname is already in use";
			break;
		case ERR_USERISREGISTERED:
			errorMsg = "462 :You may not reregister";
			break;
		case ERR_NEEDMOREPARAMS:
			errorMsg = "461 " + param + " :Not enough parameters";
			break;	
		default:
			errorMsg = "400 :Unknown error";
			break;
	}
	errorMsg += "\r\n";
	send(fd, errorMsg.c_str(), errorMsg.length(), 0);

}