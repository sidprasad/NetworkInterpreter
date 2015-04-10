#Networks - Comp 112 Final Project
### Shea Clark-Tiesche, Siddhartha Prasad, and Nikhil Shinday

This repository contains a *uScheme* interpreter that can be used by several
clients across a network. All clients share an environment, and need to be
connected to the parent server to access it.


## Current Issues

- Currently, while a clients can send messages to the server, there is an
issue with sending messages back to each client from the server. This is an
Operating Systems 'dup'ing issue with a non-trivial best effort solution.

## Current Protocol

When the server communicates with the client (and vice versa), we have the
following basic protocol.

Type   | From  | To   | Meaning|
------ |-------|------|--------
1      |Client |Server|Request to connect|
2      |Server |Client|Update to environment|
3      |Client |Server|Interpreter command|
4      |Server |Client|Interpreter command ACK |

## Future Plans

- Better client design
- Possibly distributed system
- Local environment

## Usage

Client

    gcc -g client.c -o client -lnsl
    ./client <hostname> <portnumber>

Server

    gcc -g iserv.c -o server -lnsl
    ./server <portnumber>







