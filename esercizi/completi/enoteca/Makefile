client.o: client.c definitions.h
	gcc -c client.c -DDEBUG -g

client: client.o
	gcc -o client client.o

azienda.o: azienda.c definitions.h
	gcc -c azienda.c -DDEBUG -g

azienda: azienda.o
	gcc -o azienda azienda.o

server.o: server.c definitions.h
	gcc -c server.c -DDEBUG -g

server: server.o
	gcc -o server server.o

s: server
	./server 2000

c: client
	./client ::1 2000

a: azienda
	./azienda ::1 2000 "NomAzienda"

clean:
	rm -f *.o client server azienda