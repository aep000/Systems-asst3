all: bankserv.c
	gcc bankserv.c -o server -lpthread -lm -g
server:
	gcc bankserv.c -o server -lpthread -lm -g
client: bankclient.c
	gcc bankclient.c -o client -lpthread -lm -g
clean-client:
	$(RM) client
clean-server:
	$(RM) server
clean:
	$(RM) server
	$(RM) client
