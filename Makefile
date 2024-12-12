nm:
	gcc -o nm naming_server.c tries.c funcs.c LRU.c -lpthread -lrt

ss:
	gcc -o ss storage_server.c linkedlist.c funcs.c -lpthread -lrt

clt:
	gcc -o clt client.c funcs.c