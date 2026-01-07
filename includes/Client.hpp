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
		std::string _nickname;
		std::string _inbuf;
	public:
		Client(int fd, int port, const std::string& ip);
		~Client();

		void appendInput(const char* data, size_t n);
		bool extractLine(std::string &line);

		int getFd() const;
		int getPort() const;
		std::string getIp() const;
		void setName(const std::string& name);
		std::string getName() const;
		void setNickname(const std::string& nickname);
		std::string getNickname() const;
		size_t inbufSize() const;
};

#endif
