#ifndef SERVER_HPP
#define SERVER_HPP

#include "IrcNumeric.hpp"
#include <string>
#include <csignal>
#include <map>
#include <vector>
#include <set>

class Client;
class Command;

struct IrcMessage {
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
};


class Server {
	private:
		int								_port;
		std::string						_password;
		int								_listenFd;
		std::map<int, Client>			_clients;
		std::map<std::string, Command*> _commands;
		std::set<int> 					_pendingDisconnect;

		Server(const Server& other);
		void acceptClient(int cfd, const std::string& ip, int port);
		void initCommands();
		void destroyCommands();
	public:
		Server(int port, const std::string& password);
		~Server();

		const std::map<int, Client>& getClients() const;
		int getClientFd(const Client& client) const;
		bool recvFromClient(int fd);
		void cleanup();
		void run();
		void handleLine(int fd, const std::string &line);
		void dispatchCommand(int fd, Client &client, const IrcMessage &msg);
		void sendError(int fd, int code, const std::string &param) const;

		void sendToClient(int fd, const std::string &msg) const;
		bool isRegistered(const Client &c) const;
		bool isNickTaken(const std::string &nickname, int exceptFd) const;

		std::string getServerName() const;
		bool passwordRequired() const;
		const std::string& getPassword() const;
		void tryRegister(int fd, Client &client);
		void scheduleDisconnect(int fd);
		int findClientFdByNick(const std::string &nickname) const;
};


#endif
