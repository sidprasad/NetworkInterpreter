#Networks - Comp 112 Final Project
### Shea Clark-Tiesche, Siddhartha Prasad, and Nikhil Shinday

10th April 2015

This repository/directory contains a *uScheme* interpreter that can be used by
several clients across a network. All clients share an environment, and need to
 be connected to the parent server to access it.

    Github repository: https://github.com/sidprasad/NetworkingInterpreter


## Current Issues

- Currently, while a clients can send messages to the server, there is an
issue with sending messages back to each client from the server. This is an
Operating Systems 'dup'ing issue that works only ~80% of the time.
Unfortunately we discovered this later in the day and have decided to revert
to an earlier version, where the interpreter publishes its results on the
server, sending a placeholder message coupled with its response to the client.

- Currently using a simple ACKless client for testing purposes. Eventual
client will look like client.c

## Current Protocol

When the server communicates with the client (and vice versa), we have the
following basic protocol.

Message Type   | From  | To   | Meaning|
---------------|-------|------|--------
1              |Client |Server|Request to connect|
2              |Server |Client|Update to environment|
3              |Client |Server|Interpreter command|
4              |Server |Client|Interpreter command ACK |
5              |Client |Server|Graceful exit|


A graceful exit provides the user with a copy of the interpreter as
well as a log with all the commands provided to the interpreter.
This can be used by a client to create an offline interpreter with the
same current environment.

## Future Plans

- Better client design
- Possibly distributed system
- Local environment
- Possibility of being able to gracefully disconnect with an interpreter
that can work offline.

## Usage

Client

    gcc -g client.c -o client -lnsl
    ./client <hostname> <portnumber>

Server

    gcc -g iserv.c -o server -lnsl
    ./server <portnumber>







