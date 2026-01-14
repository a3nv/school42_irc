#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <iostream>
#include <string>

class Server;

// Base Command class
class Command {
    protected:
        std::string _name;
    public:
        Command(const std::string& name);
        virtual ~Command();
};

//--------------------Connection & Registration--------------------
// NICK Command derived class
class Nick : public Command {
    public:
        Nick();
        ~Nick();
        bool validate(const std::string& nickname, const Server& server);
};

// USER Command derived class
class User : public Command {
    public:
        User();
        ~User();
};

// PASS Command derived class
class Pass : public Command {
    public:
        Pass();
        ~Pass();
};

//--------------------Messaging--------------------

// PRIVMSG Command derived class
class PrivMsg : public Command {
    public:
        PrivMsg();
        ~PrivMsg();
};

//--------------------CHANNEL--------------------
// JOIN Command derived class
class Join : public Command {
    public:
        Join();
        ~Join();
};

// PART Command derived class
class Part : public Command {
    public:
        Part();
        ~Part();
};

//--------------------DISCONNECT---------------------
// QUIT Command derived class
class Quit : public Command {
    public:
        Quit();
        ~Quit();
};

//----------------Keep Alive----------------------------
// PING Command derived class
class Ping : public Command {
    public:
        Ping() : Command("PING") {}
        ~Ping() {}
};

// PONG Command derived class
class Pong : public Command {
    public:
        Pong();
        ~Pong();
};

//------------------Channel Operator ------------------

// MODE(+i +t +k +o +l) Command derived class
class Mode : public Command {
    public:
        Mode();
        ~Mode();
};

// TOPIC Command derived class
class Topic : public Command {
    public:
        Topic();
        ~Topic();
};

// KICK Command derived class
class Kick : public Command {
    public:
        Kick();
        ~Kick();
};

// INVITE Command derived class
class Invite : public Command {
    public:
        Invite();
        ~Invite();
};

#endif