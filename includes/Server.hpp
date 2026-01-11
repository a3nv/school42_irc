#ifndef SERVER_HPP
#define SERVER_HPP

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
		int								_port;
		std::string						_password;
		int								_listenFd;
		std::map<int, Client>			_clients;
		std::map<std::string, Command*> _commands;

		Server(const Server& other);
		void acceptClient(int cfd, const std::string& ip, int port);
		void initCommands();
		void destroyCommands();
	public:
		Server(int port, const std::string& password);
		~Server();

		const std::map<int, Client>& getClients() const; 
		bool recvFromClient(int fd);
		void cleanup();
		void run();
		void handleLine(int fd, const std::string &line);
		void sendToClient(int fd, const std::string &msg);
		bool isRegistered(const Client &c) const;
};


#endif
