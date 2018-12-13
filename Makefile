all: bankserv.c bankclient.c
	gcc bankserv.c -o bankingServer -lpthread -lm -g
	gcc bankclient.c -o bankingClient -lpthread -lm -g
server: bankserv.c
	gcc bankserv.c -o bankingServer -lpthread -lm -g
client: bankclient.c
	gcc bankclient.c -o bankingClient -lpthread -lm -g
clean-client:
	$(RM) client
clean-server:
	$(RM) server
clean:
	$(RM) server
	$(RM) client
