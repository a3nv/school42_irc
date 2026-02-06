This project has been created as part of the 42 curriculum by iasonov and relgheit
# Description:
This project is an implementation of a simplified IRC (Internet Relay Chat) server compliant with the core IRC protocol. The goal is to build a fully functional server capable of handling multiple clients, managing user connections, and supporting real-time text communication.

The server implements the essential IRC commands, including user registration, private messaging, and channel management. Users can connect to the server using a standard IRC client, communicate directly with other users, or join channels to participate in group discussions. Channel-related features such as joining, leaving, operator management, and basic channel modes are supported.

# Instructions:

### 1. build application
make

### 2. run
./ircserv port_number password

### 3. connect
nc -v IP/localhost port_number

### 4. Register client
PASS password

USER username * 0 :username

NICK nickname

#### 5. Communicate

PRIVMSG user :message

### 6. Channels
Join #channel

PRIVMSG #channel :message

PART #channel,{channels}

### 7. Operators
#### Invite only 
MODE #channel +i
#### Set Password
MODE #channel +k password
#### Assign operator
MODE #channel +o user
#### Set users limit
MODE #channel +l number
#### Restirct changing topic to only operators
MODE #channel +t
#### Kick user from channel
KICK #channel user


# Resources:
[Modern IRC Client Protocol](https://modern.ircdocs.horse/)

[RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459)

[Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/)