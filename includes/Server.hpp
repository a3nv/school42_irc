#ifndef SERVER_HPP
#define SERVER_HPP

#include "IrcNumeric.hpp"
#include "Channel.hpp"
#include <string>
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
		std::map<std::string, Channel>	_channels;

		Server(const Server& other);
		void acceptClient(int cfd, const std::string& ip, int port);
		void initCommands();
		void destroyCommands();

		static bool setNonBlocking(int fd);
		bool flushClientOutput(int fd);

		static std::string normalizeName(const std::string &s);
		std::string normalizeChanKey(const std::string &chan) const;

		Channel* findChannelByKey(const std::string &key);
		Channel& getOrCreateChannel(const std::string &displayName);
		void removeClientFromAllChannels(int fd, const std::string &quitMsg);

		void broadcastToChannel(Channel &ch, const std::string &line, int exceptFd);

		std::string makePrefix(const Client &c) const;

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

		void sendNumeric(int fd, int code, const std::vector<std::string> &params, const std::string &trailing);
		void sendError(int fd, int code, const std::string &param);

		void sendToClient(int fd, const std::string &msg);

		bool isRegistered(const Client &c) const;
		bool isNickTaken(const std::string &nickname, int exceptFd) const;

		std::string getServerName() const;
		bool passwordRequired() const;
		const std::string& getPassword() const;

		void tryRegister(int fd, Client &client);
		void scheduleDisconnect(int fd);

		int findClientFdByNick(const std::string &nickname) const;

		// Channel helpers used by commands
		bool isChannelName(const std::string &target) const;
		void joinChannel(int fd, Client &client, const std::string &chanName, const std::string &keyArg);
		void partChannel(int fd, Client &client, const std::string &chanName, const std::string &partMsg);
		void sendNames(int fd, Client &client, const std::string &chanName);
		void channelPrivmsg(int fd, Client &client, const std::string &chanName, const std::string &text);
		void setChannelTopic(int fd, Client &client, const std::string &chanName, const std::string &topic);
		void getChannelTopic(int fd, Client &client, const std::string &chanName);
		void channelMode(int fd, Client &client, const IrcMessage &msg);
		void channelInvite(int fd, Client &client, const std::string &nick, const std::string &chanName);
		void channelKick(int fd, Client &client, const std::string &chanName, const std::string &nick, const std::string &reason);
};

#endif
