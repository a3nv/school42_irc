# school42_irc

## Roadmap 

Highlevel roadmap would look like:
- use an existing irc client e.g. [irssi](docs/irssi.md)
- connect to some existing server e.g. [libera](docs/libera.md)
- try it out and understand what are the commands and how they work
- start implementing own server and use irssi as a test tool against it
- run server in a container and try connecting from host machine?

### Current state (todos)
- [ ] server error handling
- [ ] store clients instead of closing them right away
- [ ] getting to that poll implementation

## How to test connection

Dev container has both `ss` and `netstat` to test what is running on a specific port. Also it exposes 4444 port so we can connect to the server from outside of container as long as we use exposed port.

> [!NOTE]
> When running devcontainer vs code can auto-forward port (see ports tab -> ctrl + `)


```bash
# 1. build application
make

# 2. run application 
./ircserv 4444 test

# 3. check what is running on the port in the container
ss -ltnp | grep 4444
# or
netstat -ltnp | grep 4444


# 4. connect
nc -zv 127.0.0.1 3333 # attempts to connect and immediately return signal
nc -v 127.0.0.1 4444
```

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

#5. once client is connected you can simply type a message in a client terminal it should be delivered to server
```

## Testing commands
```bash
PING :server1.example.net

NICK John
USER john 0 * :John Doe

JOIN #42prague
PART #42prague :gotta go
QUIT :bye for now

PRIVMSG #general :hello there friends


#prefix
:irc.example.net 001 john :Welcome to the Internet Relay Network john!john@host

:nick!user@host PRIVMSG #general :hi

:nick!user@host JOIN #general

:nick!user@host QUIT :Connection reset by peer

# rest
MODE #general +o john
TOPIC #general :Roadmap discussion at 18:00
KICK #general troublemaker :spamming links
NOTICE john :*** Looking up your hostname...


#failed
               # empty spaces
:irc.example.net
:irc.example.net     \r\n
: #only colon
# weird spacing
PRIVMSG    #chan     :hello     there     

```