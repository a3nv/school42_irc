#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <iostream>
#include <string>

// Base Command class
class Command {
    protected:
        std::string _name;
    public:
        Command(const std::string& name);
        ~Command();
};

//--------------------Connection & Registration--------------------
// NICK Command derived class
class Nick : public Command {
    public:
        Nick() : Command("NICK") {}
        ~Nick() {}
};

// USER Command derived class
class User : public Command {
    public:
        User() : Command("USER") {}
        ~User() {}
};

// PASS Command derived class
class Pass : public Command {
    public:
        Pass() : Command("PASS") {}
        ~Pass() {}
};

//--------------------Messaging--------------------

// PRIVMSG Command derived class

//--------------------CHANNEL--------------------
// JOIN Command derived class

// PART Command derived class

//--------------------DISCONNECT---------------------
// QUIT Command derived class

//----------------Keep Alive----------------------------
// PING Command derived class

// PONG Command derived class

//------------------Channel Operator ------------------

// MODE(+i +t +k +o +l) Command derived class

// TOPIC Command derived class

// KICK Command derived class

// INVITE Command derived class


#endif