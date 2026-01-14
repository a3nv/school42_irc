#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <csignal>
#include <map>
#include <vector>

class Client;
class Command;

enum {
	ERR_UNKNOWNCOMMAND = 421,
	ERR_NONICKNAMEGIVEN = 431,
	ERR_ERRONEUSNICKNAME = 432,
	ERR_NICKNAMEINUSE = 433,
	RPL_WELCOME = 001
};

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
		void sendError(int fd, int code);
		void handleNick(int fd, const IrcMessage &msg);
};


#endif
