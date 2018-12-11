all: bankserv.c bankclient.c
	gcc bankserv.c -o server -lpthread -lm -g
	gcc bankclient.c -o client -lpthread -lm -g
server: bankserv.c
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
