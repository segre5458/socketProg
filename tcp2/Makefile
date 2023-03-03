CFLAGS := -Wall -Wextra 
.PHONY: all
all: server client
server: server.c common.c
	gcc $^ -o $@ $(CFLAGS)  
client: client.c common.c
	gcc $^ -o $@ $(CFLAGS)  

