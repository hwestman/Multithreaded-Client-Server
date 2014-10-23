CFLAGS = -g -std=c++0x
FILES = main.o Client.o Server.o Init.o SemLock.o
all:$(FILES)
	g++ $(CFLAGS) $(FILES) -o program
	
main.o: main.cpp common.h Headers.h
	cc $(CFLAGS) -c main.cpp -c
	
Client.o: Client.cpp common.h Headers.h
	cc $(CFLAGS) -c Client.cpp -c
	
Server.o: Server.cpp common.h Headers.h
	cc $(CFLAGS) -c Server.cpp -c
	
Init.o: Init.cpp common.h Headers.h
	cc $(CFLAGS) -c Init.cpp -c
	
SemLock.o: SemLock.cpp SemLock.h common.h Headers.h
	cc $(CFLAGS) -c SemLock.cpp -c
	
clean:
	rm -rf *exe
	rm -rf *.o
	rm -rf *.out
	rm -rf program