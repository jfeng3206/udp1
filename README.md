# Programming Assignment 1 README

## Overview

This socket program is using customized protocol on top of UDP protocol for sending information to the server.

- The client firstly sends five packets to the server.

- The server acknowledges with ACK receive of each correct packet from client.

- The client then sends another five packets to the server, emulating one correct packet and four packets with errors.

- The server acknowledges with ACK receive of correct packet from client, and with corresponding Reject sub codes for packets with errors.

- The client retransmit the packet if the ACK for each packet has not been received before expiration of timer (3 seconds).

- If no ACK was received from the server after resending the same packet 3 times, the client will stop the process.

## Setup and Usage

1. **Server**
   ```
   cd ~/server
   cc -o server server.c
   ./server
   ```

2. **Client**
   ```
   cd ~/client
   cc -o client client.c
   ./client
   ```
## File Structure

- `client/`: Contains client's source code, and datapacket.txt that contains 10 packets the client will be sending.
- `server/`: Contains server's source code.
- `README.md`: This file.
- `client_err_msg.png`: simulated client error response messages displayed
- `server_err_msg.png`: simulated server error response messages displayed
