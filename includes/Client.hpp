#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
// Client class definition
class Client {
    private:
        std::string _name;
        std::string _nickname;
    public:
        Client(); // Constructor
        ~Client(); // Destructor
        void setName(const std::string& name);
        void setNickname(const std::string& nickname);
        std::string getName() const;
        std::string getNickname() const;
};

#endif