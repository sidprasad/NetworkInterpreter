#Networks - Comp 112 Final Project
### 10th April 2015

This repository contains a *uScheme* interpreter that can be used by several
clients across a network. All clients share an environment, and need to be
connected to the parent server to access it.


## Current Issues

- Currently, while a clients can send messages to the server, there is an
issue with sending messages back to each client from the server. This is an
Operating Systems 'dup'ing issue with a non-trivial best effort solution.

## Current Protocol


## Future Plans


Todo:

- Build client
- Choose better interpreter
- Eliminate activitylist and idlist



