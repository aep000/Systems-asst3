all: bankserv.c
	gcc bankserv.c -o server -lpthread -lm -g
server:
	gcc bankserv.c -o server -lpthread -lm -g
clean:
	$(RM) server
