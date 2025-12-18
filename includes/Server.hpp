#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>

class Server {
private:
    int _port;
    std::string _password;
    int _listenFd;

    Server(const Server& other);
    Server& operator=(const Server& other);

public:
    Server(int port, const std::string& password);
    ~Server();

    void run();
};


#endif // SERVER_HPP