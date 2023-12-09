client.o: client.c definitions.h
	gcc -c client.c -g

client: client.o
	gcc -o client client.o

server.o: server.c definitions.h
	gcc -c server.c -g

server: server.o
	gcc -o server server.o

s: server
	./server 2000

c: client
	./client ::1 2000

clean:
	rm -f *.o client server