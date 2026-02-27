_This project has been created as part of the 42 curriculum by iasonov, relgheit_

## Description:
This project is an implementation of a simplified IRC (Internet Relay Chat) server compliant with the core IRC protocol. The goal is to build a fully functional server capable of handling multiple clients, managing user connections, and supporting real-time text communication.

The server implements the essential IRC commands, including user registration, private messaging, and channel management. Users can connect to the server using a standard IRC client, communicate directly with other users, or join channels to participate in group discussions. Channel-related features such as joining, leaving, operator management, and basic channel modes are supported.

## Instructions:

### 1. build application
`make`

### 2. run
`./ircserv port_number password` 

Below commands might help verify that the app is running:

```bash
check what is running on the port in the container
ss -ltnp | grep 4444
# or
netstat -ltnp | grep 4444
```

### 3. connect
`nc -v IP/localhost port_number` e.g. `nc -v 127.0.0.1 4444`

Expected output should be something like
```bash
root@529d3e23c7a3:/workspaces/school42_irc# ./ircserv 3333 test
Starting server on port: 3333
Listening on port 3333
Client connected from 127.0.0.1:34320
Client connected from 127.0.0.1:60992
Client connected from 127.0.0.1:52888
Client connected from 127.0.0.1:52898
Client connected from 127.0.0.1:39826
```

### 4. Register a client

```bash
PASS password
USER username * 0 :username
NICK nickname
```

### 5. Communicate

`PRIVMSG user :message`

### 5.1 Testing various commands

#### Ping

```bash
PING :server1.example.net
:irc42 PONG irc42 :server1.example.net
```

### 6. Channels

```bash
Join #channel
PRIVMSG #channel :message
PART #channel,{channels}
```

### 7. Operators

#### Invite only 
`MODE #channel +i`
#### Set Password
`MODE #channel +k password`
#### Assign operator
`MODE #channel +o user`
#### Set users limit
`MODE #channel +l number`
#### Restirct changing topic to only operators
`MODE #channel +t`
#### Kick user from channel
`KICK #channel user`


### Testing scenarios

```bash

$ nc -v localhost 4444

# connect first user 
$ pass 1234
$ NICK John
$ USER john 0 * :John Doe
:irc42 001 John :Welcome to the Internet Relay Network John!john@192.168.65.1
:irc42 002 John :Your host is irc42, running version 0.1
:irc42 003 John :This server was created today
:irc42 004 John :irc42 0.1

# connect second user 
$ nc -v localhost 4444
Connection to localhost port 4444 [tcp/krb524] succeeded!
NOTICE AUTH :*** Looking up your hostname...
NOTICE AUTH :*** Checking ident...
NOTICE AUTH :*** Welcome! Please register with PASS/NICK/USER
$ pass 1234
$ nick bob
$ user bob 0 * :Robert Rumbles
:irc42 001 bob :Welcome to the Internet Relay Network bob!bob@192.168.65.1
:irc42 002 bob :Your host is irc42, running version 0.1
:irc42 003 bob :This server was created today
:irc42 004 bob :irc42 0.1

# send a message from John
$ privmsg bob :hello m8
# bob should receive 
:John!john@192.168.65.1 PRIVMSG bob :hello m8

# send a notice from Bob
$ notice john :salute!
# John should receive
:bob!bob@192.168.65.1 NOTICE john :salute!

# join a channel with both clients
$ JOIN #room 
:John!john@192.168.65.1 JOIN :#room
:irc42 331 John #room :No topic is set
:irc42 353 John = #room :@John
:irc42 366 John #room :End of /NAMES list.
:bob!bob@192.168.65.1 JOIN :#room

# send message to a channel
$ PRIVMSG #room :hello from bob
:bob!bob@192.168.65.1 PRIVMSG #room :hello from bob

# Leave the channel
$ PART #room
:bob!bob@192.168.65.1 PART #room :Leaving ## an entry in bob's chat

## Message again to the channel bob should not receive anything

:bob!bob@192.168.65.1 PART #room :Leaving ## an entry in John's chat 
$ PART #room
:John!john@192.168.65.1 PART #room :Leaving

## Note! Case sensitive channels still the same channel

## Connect from another client. NOTE! This also handles CAP command verification
irssi
/connect 127.0.0.1 6667 1234
/join #room
/msg #room hello from sally
```

```bash
MODE #test +i
INVITE bob #test
MODE #test +t
TOPIC #test :Only ops can set topic now
MODE #test +l 1
KICK #test bob :bye
```

### WeeChat

```bash
# run wechat
weechat

# register server
/server add irc42 127.0.0.1/4444 -password=1234

# connect to the server
/connect irc42

/set irc.server.irc42.nicks John
/set irc.server.irc42.username john
/set irc.server.irc42.realname "John Doe"

/disconnect irc42
/connect irc42

/join #room

/msg #room hello from weechat

/msg bob hey bob
```

## Resources:
- [Modern IRC Client Protocol](https://modern.ircdocs.horse/)
- [RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/)

### AI usage disclosure
AI tools were used as a learning and productivity aid (not as a code generator for the final solution):
- To better understand IRC concepts and real server behavior “under the hood” (registration flow, numerics, message formatting, CAP negotiation basics, typical client expectations).
- To suggest non-obvious/manual test scenarios for the implemented commands (edge cases around registration, channel membership, modes, operator actions, and message routing).
- To speed up manual verification using real IRC clients (irssi and weechat), including command sequences to reproduce scenarios quickly.
- To do basic sanity checks on protocol formatting and validation rules (CRLF termination, prefix/params/trailing parsing, case handling, and numeric reply expectations).

All implementation decisions and final code were written and validated by the project authors.