# socket-server-client

The server program will establish the machine as a server listening on a port, will print a message whether it receives a packet or lost it, and will generate a random number to determine whether the packet is "lost".

The client program will establish this machine as a client, will send ping packets to the specified server and port, wait 1 second for a response, will print a message depending on whether a response is received, and will print statistics regarding the round-trip response times.