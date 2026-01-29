#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "IrcNumeric.hpp"
#include <string>

class Server;
class Client;
struct IrcMessage;

class Command {
protected:
    std::string _name;

    virtual bool requiresRegistration() const;
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg) = 0;

public:
    Command(const std::string &name);
    virtual ~Command();

    const std::string &name() const;
    void run(Server &server, int fd, Client &client, const IrcMessage &msg);
};

// --------------------Connection & Registration--------------------
class Cap : public Command {
public:
    Cap();
    virtual ~Cap();
protected:
    virtual bool requiresRegistration() const;
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

class Nick : public Command {
public:
    Nick();
    virtual ~Nick();
protected:
    virtual bool requiresRegistration() const;
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
private:
    bool validate(const std::string &nickname, Server &server, int fd) const;
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

// --------------------Messaging--------------------
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

// --------------------Channels--------------------
class Join : public Command {
public:
    Join();
    virtual ~Join();
protected:
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

class Part : public Command {
public:
    Part();
    virtual ~Part();
protected:
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

class Names : public Command {
public:
    Names();
    virtual ~Names();
protected:
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

class Mode : public Command {
public:
    Mode();
    virtual ~Mode();
protected:
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

class Topic : public Command {
public:
    Topic();
    virtual ~Topic();
protected:
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

class Invite : public Command {
public:
    Invite();
    virtual ~Invite();
protected:
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

class Kick : public Command {
public:
    Kick();
    virtual ~Kick();
protected:
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

// --------------------Disconnect---------------------
class Quit : public Command {
public:
    Quit();
    virtual ~Quit();
protected:
    virtual bool requiresRegistration() const;
    virtual void execute(Server &server, int fd, Client &client, const IrcMessage &msg);
};

// ----------------Keep Alive----------------------------
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

#endif
