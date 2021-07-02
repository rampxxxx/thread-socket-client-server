# thread-socket-client-server
TCP/IP client, TCP/IP server and some very basic message based protocol. 
The code must fit next conditions:
Both client and server must be written on C for Linux
Client and server should communicate via TCP/IP protocol
The client app must be multithread (use libpthread)
The client app runs N threads, each thread establishing connection to the server 
app and in loop send 5 millions messages to the server and exits after all 
messages acknowledged by server. Size of a message is 8KB, message has some 
header and body, a message body can contain non-zero data. The client must hold 
sent messages in memory until acknowledge received.
The client app exits after all threads finished,  size of memory allocated from 
heap must not exceed 2GB.
The server app is also multithread.  It has single receiver thread, and N/3 
worker threads which stores received data in N files (1 file per client). 
After data successfully stored on a disk the server must send ack to the 
client. Note that the server must not lost acknowledged data in case of server 
crash/power lost and etc.
N is constant defined in header and equal to 64.
You can skip error handling, but should pay attention to performance, latency, 
memory consumption and etc. The main target is to minimize execution time of 
client.
