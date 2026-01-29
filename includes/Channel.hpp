#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

class Channel {
private:
    std::string _name;
    std::string _key;
    std::set<int> _members;
    std::set<int> _ops;

    bool _inviteOnly;         // +i
    bool _topicOpOnly;        // +t
    std::string _keyPassword; // +k
    bool _hasKey;
    size_t _limit;            // +l
    bool _hasLimit;

    std::set<int> _invited;

    std::string _topic;
    bool _hasTopic;

public:
    Channel();
    Channel(const std::string &displayName, const std::string &key);

    const std::string &name() const;
    const std::string &key() const;

    // membership
    bool hasMember(int fd) const;
    void addMember(int fd);
    void removeMember(int fd);
    const std::set<int> &members() const;
    bool empty() const;

    // ops
    bool isOp(int fd) const;
    void addOp(int fd);
    void removeOp(int fd);
    const std::set<int> &ops() const;

    // invites
    bool isInvited(int fd) const;
    void invite(int fd);
    void uninvite(int fd);

    // modes
    bool inviteOnly() const;
    void setInviteOnly(bool v);

    bool topicOpOnly() const;
    void setTopicOpOnly(bool v);

    bool hasKey() const;
    const std::string &keyPassword() const;
    void setKeyPassword(const std::string &k);
    void clearKeyPassword();

    bool hasLimit() const;
    size_t limit() const;
    void setLimit(size_t v);
    void clearLimit();

    // topic
    bool hasTopic() const;
    const std::string &topic() const;
    void setTopic(const std::string &t);
    void clearTopic();

    std::string modeString() const; // e.g. "+itkl"
};

#endif
