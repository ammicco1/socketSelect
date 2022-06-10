#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXLEN 255
#define MAXCONN 5

void die(char *);

int main(int argc, char **argv){
	if(argc < 3){
		die("no port specified\nyou must specify:\n - UDP port on argv 1\n - TCP port on argv 2");
	}	

    int i;

    /* variabili per la select */
    fd_set readsd;
    int n, maxsd;
    struct timeval tv; 

    /* variabili per il server UDP */
    int port_udp= atoi(argv[1]);
    int sd_udp;
    struct sockaddr_in bind_ip_port_udp, client_ip_port_udp;
    int bind_ip_port_length_udp = sizeof(bind_ip_port_udp), 
    client_ip_port_length_udp = sizeof(client_ip_port_udp);
    char rec_udp[MAXLEN], send_udp[MAXLEN];
    int byte_rec_udp, byte_send_udp;

    /* variabili per il server TCP */
    int port_tcp = atoi(argv[2]);
    int sd_tcp;
    struct sockaddr_in bind_ip_port_tcp, client_ip_port_tcp;
    int bind_ip_port_length_tcp = sizeof(bind_ip_port_tcp), 
    client_ip_port_length_tcp = sizeof(client_ip_port_tcp);
    int tcp_conn_sd;
    int byte_rec_tcp, byte_send_tcp;
    char rec_tcp[MAXLEN], send_tcp[MAXLEN];

    /* azzero buffer */
    memset(rec_tcp, 0, MAXLEN);
    memset(rec_udp, 0, MAXLEN);

    sd_udp = socket(AF_INET, SOCK_DGRAM, 0);
    sd_tcp = socket(AF_INET, SOCK_STREAM, 0);

    if(sd_tcp < 0 || sd_udp < 0){
        die("socket() error");
    }

    printf("socket() ok.\n");

    /* set ip and port for udp server */
    bind_ip_port_udp.sin_family = AF_INET;
	bind_ip_port_udp.sin_addr.s_addr = inet_addr("127.0.0.1");
	bind_ip_port_udp.sin_port = htons(port_udp);

    /* set ip and port for tcp server */
    bind_ip_port_tcp.sin_family = AF_INET;
	bind_ip_port_tcp.sin_addr.s_addr = inet_addr("127.0.0.1");
	bind_ip_port_tcp.sin_port = htons(port_tcp);

    if(bind(sd_udp, (struct sockaddr *) &bind_ip_port_udp, bind_ip_port_length_udp) < 0){
        die("bind() udp error");
    }else if(bind(sd_tcp, (struct sockaddr *) &bind_ip_port_tcp, bind_ip_port_length_tcp) < 0){
        die("bind() tcp error");
    }

    printf("bind() ok\n");

    /* numero max sessioni tcp*/
    if(listen(sd_tcp, MAXCONN) != 0){
		die("listen() error");
	}

	printf("listen() ok.\n");

    /* inzializzo tv */
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    /* setto il massimo socket descriptor */
    maxsd = sd_udp;
    maxsd = (maxsd > sd_tcp ? maxsd : sd_tcp);

    printf("\nUDP: listen on port %d\nTCP: listen on port %d\n\n", port_udp, port_tcp);

    /* inizializzo readsd */
    FD_ZERO(&readsd);
    
    while(1){
        FD_SET(sd_udp, &readsd);
        FD_SET(sd_tcp, &readsd);
        
        n = select(maxsd + 1, &readsd, NULL, NULL, &tv);

        if(n == -1){
            die("select() error");
        }
        
        if(FD_ISSET(sd_udp, &readsd)){
            int byte_rec_udp = recvfrom(sd_udp, rec_udp, MAXLEN, 0, (struct sockaddr *) &client_ip_port_udp, &client_ip_port_length_udp);
	        
            if(byte_rec_udp <= 0){
                die("UDP socket: recvfrom() error");
            } 
	        
            printf("UDP socket: %d byte received: %s.\n", byte_rec_udp, rec_udp);
		
	        strcpy(send_udp, rec_udp);
	
	        int byte_send_udp = sendto(sd_udp, send_udp, MAXLEN, 0, (struct sockaddr *) &client_ip_port_udp, client_ip_port_length_udp);
	        
            if(byte_send_udp <= 0){
                die("UDP socket: sendto() error");
            } 
	        
            printf("UDP socket: %d byte sent.\n", byte_send_udp);
        }else if(FD_ISSET(sd_tcp, &readsd)){
            tcp_conn_sd = accept(sd_tcp, (struct sockaddr *) &client_ip_port_tcp, &client_ip_port_length_tcp);
            
            if(tcp_conn_sd < 0){
		        die("TCP socket: accept() error");
		    }

		    printf("TCP socket: accept() ok.\n");

            byte_rec_tcp = read(tcp_conn_sd, rec_tcp, MAXLEN);

			if(byte_rec_tcp<= 0){
			    die("TCP socket: read() error");
			}

			printf("TCP socket: read() ok.\nTCP socket: %d byte received: %s\n", byte_rec_tcp, rec_tcp);
		
			strcpy(send_tcp, rec_tcp);

			/* spedisco i dati al client */
			byte_send_tcp = write(tcp_conn_sd, send_tcp, MAXLEN);

			if(byte_send_tcp <= 0){
			    die("TCP socket: send() error");
		    }

			printf("TCP socket: send() ok.\n");           
        }
    }

    close(sd_udp);
    close(sd_tcp);

    return 0; 
}

void die(char *error){
	fprintf(stderr, "%s.\n", error);
	
	exit(1);
}
