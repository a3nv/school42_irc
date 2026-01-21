#ifndef SERVER_HPP
#define SERVER_HPP

#include "IrcNumeric.hpp"
#include "Channel.hpp"
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
		std::map<std::string, Channel> _channels;

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

		// Channel
		static std::string normalizeChanKey(const std::string &name);
		bool channelExists(const std::string &key) const;
		bool isClientInChannel(const std::string &key, int fd) const;
		void joinChannel(const std::string &key, const std::string &displayName, int fd);
		void partChannel(const std::string &key, int fd);
		void removeFromAllChannels(int fd);
		void sendToChannel(const std::string &key, const std::string &line, int exceptFd) const;
		std::string getChannelDisplayName(const std::string &key) const;
		std::string getChannelDisplayNameOrRaw(const std::string &key, const std::string &raw) const;
		void sendNamesReply(int fd, const Client &requester, const std::string &key) const;
};


#endif
