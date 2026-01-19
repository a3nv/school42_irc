#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "IrcNumeric.hpp"
#include <iostream>
#include <string>

class Server;
struct IrcMessage;

// Base Command class
class Server;
class Client;
struct IrcMessage;

class Command {
protected:
    std::string _name;

    // Overrides in derived commands if they are usable before registration.
    virtual bool requiresRegistration() const;

    // Implements command-specific logic.
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg) = 0;

public:
    Command(const std::string &name);
    virtual ~Command();

    const std::string &name() const;

    // Entry point called by Server.
    void run(Server &server, int fd, Client &client, const IrcMessage &msg);
};

//--------------------Connection & Registration--------------------
class Nick : public Command {
public:
    Nick();
    virtual ~Nick();

protected:
    virtual bool requiresRegistration() const;
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);

private:
    bool validate(const std::string &nickname, const Server &server, int fd) const;
    static bool isValidNickname(const std::string &nickname);
};

class User : public Command {
public:
    User();
    virtual ~User();
protected:
    virtual bool requiresRegistration() const;
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

class Pass : public Command {
public:
    Pass();
    virtual ~Pass();
protected:
    virtual bool requiresRegistration() const;
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

//--------------------Messaging--------------------

// PRIVMSG Command derived class
class PrivMsg : public Command {
public:
    PrivMsg();
    virtual ~PrivMsg();
protected:
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

class Notice : public Command {
public:
    Notice();
    virtual ~Notice();
protected:
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
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
class Quit : public Command {
public:
    Quit();
    virtual ~Quit();
protected:
    virtual bool requiresRegistration() const;
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

//----------------Keep Alive----------------------------
class Ping : public Command {
public:
    Ping();
    virtual ~Ping();

protected:
    virtual bool requiresRegistration() const;
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

class Pong : public Command {
public:
    Pong();
    virtual ~Pong();
protected:
    virtual bool requiresRegistration() const;
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
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
