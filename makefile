ARGS = -Wall -ansi -pedantic

server client_tcp client_udp: server.c client_tcp.c client_udp.c
	gcc ${ARGS} -o server server.c
	gcc ${ARGS} -o client_tcp client_tcp.c
	gcc ${ARGS} -o client_udp client_udp.c

clean: 
	rm server client_tcp client_udp