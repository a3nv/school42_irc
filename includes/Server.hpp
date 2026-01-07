#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <csignal>
#include <map>

class Client; 

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
};


#endif
