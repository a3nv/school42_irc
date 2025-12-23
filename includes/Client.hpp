#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
// Client class definition
class Client {
    private:
        const int _fd;
        int _port;
        std::string _ip;
        std::string _name;
        std::string _nickname;
    public:
        Client(int fd, int port, const std::string& ip); // Constructor
        ~Client(); // Destructor
        void setName(const std::string& name);
        void setNickname(const std::string& nickname);

        std::string getName() const;
        std::string getNickname() const;
        std::string getIp() const;
        int getPort() const;
        int getFd() const;
};

#endif