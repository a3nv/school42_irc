#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

class Channel {
private:
    std::string _name;
    std::set<int> _members;

public:
    Channel();
    Channel(const std::string &name);
    ~Channel();

    const std::string &name() const;

    void addMember(int fd);
    void removeMember(int fd);
    bool hasMember(int fd) const;
    bool empty() const;

    const std::set<int> &members() const;
};

#endif
