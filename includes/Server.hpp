#ifndef SERVER_HPP
#define SERVER_HPP

#include "IrcNumeric.hpp"
#include <string>
#include <csignal>
#include <map>
#include <vector>

class Client;
class Command;

struct IrcMessage {
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
};


class Server {
	private:
		int						_port;
		std::string				_password;
		int						_listenFd;
		std::map<int, Client>	_clients;

		Server(const Server& other);
		void acceptClient(int cfd, const std::string& ip, int port);
		// Server& operator=(const Server& other);
	public:
		Server(int port, const std::string& password);
		~Server();

		const std::map<int, Client>& getClients() const;
		bool recvFromClient(int fd);
		void cleanup();
		void run();
		void handleLine(int fd, const std::string &line);
		void dispatchCommand(int fd, const IrcMessage &msg);
		void sendError(int fd, int code, std::string param) const;

	// Command handlers	
		void handleNick(int fd, const IrcMessage &msg);
		bool uniqueNickname(const std::string& nickname, int fd) const;
		void handleUser(int fd, const IrcMessage &msg);
};


#endif
