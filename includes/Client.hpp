#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstddef>
#include <iostream>
#include <string>
#include <set>

class Client {
	private:
		const int _fd;
		int _port;
		std::string _ip;

		// USER <username>
		std::string _name;
		// USER ... :<realname>
		std::string _realname;
		std::string _nickname;

		std::string _inbuf;

		bool _hasPass;
		bool _hasNick;
		bool _hasUser;
		bool _registered;

		std::set<std::string> _channels;

	public:
		Client();
		Client(int fd, int port, const std::string& ip);
		~Client();

		void appendInput(const char* data, size_t n);
		bool extractLine(std::string &line);

		int getFd() const;
		int getPort() const;
		std::string getIp() const;

		void setName(const std::string& name);
		std::string getName() const;

		void setRealName(const std::string& realname);
		std::string getRealName() const;

		void setNickname(const std::string& nickname);
		std::string getNickname() const;

		void setHasPass(bool v);
		bool hasPass() const;

		void setHasNick(bool v);
		bool hasNick() const;

		void setHasUser(bool v);
		bool hasUser() const;

		void setRegistered(bool v);
		bool isRegistered() const;

		void joinChannel(const std::string &name);
		void partChannel(const std::string &name);
		bool isInChannel(const std::string &name) const;
		const std::set<std::string> &getChannels() const;

		size_t inbufSize() const;
};

#endif
