# ft irc

Done: No
Project: 42 school level 05 (https://www.notion.so/42-school-level-05-26e37d4cbeb880119f46fc6369394744?pvs=21)
Subject: https://www.notion.so/463636145fb74285abdff10da53649b7
Epic: https://www.notion.so/4342b668ba9848f5b3b0b695d9ec04c9

## IRSSI

https://irssi.org/New-users/

## LIBERA

https://netsplit.de/channels/?net=libera.chat&chat=java

## How to create IRC

https://beej.us/guide/bgnet/html/

## [Makefile](https://www.notion.so/Makefile-2e137d4cbeb880aca542ff1d972047ae?pvs=21)

Make sure it is using CPP ready makefile (compiler, cxx, version, flags)

## BRD

- multiple clients simultaneously without hanging
- no forking
- non-blocking I/O operations - all of them
- only one `poll()` (or equivalent `select()`) for all operations (read/write/listen/etc.)
    
    `recv/send` on fds without using `poll/select` for readiness, grade 0 
    
- must handle partial data / packet aggregation (see test example in pdf)

### Functional features to support

- [ ]  authentication (PASS command)
- [ ]  set nickname (NICK command)
- [ ]  set username (USER command)
- [ ]  join a channel (JOIN command)
- [ ]  send/receive private messages (PRIVMSG command)
- [ ]  forward channel messages to everyone in the channel
- [ ]  have operators and regular users
- [ ]  operator commands:
    - `KICK`
    - `INVITE`
    - `TOPIC`
    - `MODE` with modes: `i, t, k, o, l`
- [ ]  PING command → pong
- [ ]  quit command (cleanup)

## Roadmap

1. Phase 1 - network (goal: accept connections, read data, split data)
    - [x]  socket setup
    1. Event loop
        - [x]  select
        - [x]  client registration (network level registration)
        - [x]  read from client
        - [x]  partial data
    - [x]  printing line received from the client
2. phase 2 - received line from the client transformed into a structured IRC message and call the right handler -> Jan 17, 2026
    1. parsing (parse → normalize → store?) -> Jan 10, 2026
        1. `COMMAND params... :trailing` `:` - separated trailing part
        - [ ]  message struct?
    2. dispatching (map command → handler) -> Jan 11, 2026 - draft
        1. `pass`, `nick`, `user`, `join`, `privmsg`, `ping`, `quit`
    3. IRC registration (different from 1.1 (client registration)) (we would need some kind of a state at client level)
        - [ ]  check how it works in real IRC when client is not registered
        - [ ]  pass → then NICK/USER
        - [ ]  they can use IRC only after above steps
        - [ ]  replies/errors
3. phase 3 - complete chat behavior channels, users, topics, server replies -> 2 weeks for phase 3, expected Jan 31, 2026
    1. another map <string, Channel>
    2. Channel supposed to store members, operators, topic, 
        1. modes (i,t,k,l) required by forwarding channel message and operator commands
            1. i - invite list
            2. t - topic only
            3. k - key/pswd
            4. l - user limit
    3. Client stores registered flag, nick, user, joined channels
    4. Channel message forwarding `PRIVMSG #chan :text` 
    5. Operators → channel creator? some kind of promotion
        - `KICK`
        - `INVITE`
        - `TOPIC`
        - `MODE` with modes: `i, t, k, o, l`
    6. replies/output ??
        1. send is not blocked
        2. send only when select is writable