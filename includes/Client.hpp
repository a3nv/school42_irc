#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <cstddef>
#include <iostream>

class Client {
	private:
		const int _fd;
		int _port;
		std::string _ip;
		std::string _name;
		std::string _username;
		std::string _nickname;
		std::string _inbuf;
		bool _registered;
	public:
		Client();
		Client(int fd, int port, const std::string& ip);
		~Client();

		void appendInput(const char* data, size_t n);
		bool extractLine(std::string &line);

		int getFd() const;
		int getPort() const;
		std::string getIp() const;
		std::string getName() const;
		std::string getNickname() const;
		std::string getUsername() const;
		void setName(const std::string& name);
		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		bool tryRegisterClient();
		bool isRegistered() const;
		size_t inbufSize() const;
};

#endif
