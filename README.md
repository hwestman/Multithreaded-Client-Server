##A client and a server communicating over IPC.

###The server runs a threadpool executing arbitrary task requested by the client.

In this implementation the client takes int's from a a keyboard input and requests the server that factorize the input using a threadpool to distribute the task. 
_All results are returned async to the client._

```
make
./program <size of thread pool>

