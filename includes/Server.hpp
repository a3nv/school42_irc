#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream>


// Server class definition
class Server {
    private:
        int _port;
        std::string _password;
    public:
        Server(unsigned int port, const std::string& password); // Parameterized constructor
        ~Server(); // Destructor "Do we need canonical form?"
};


#endif // SERVER_HPP