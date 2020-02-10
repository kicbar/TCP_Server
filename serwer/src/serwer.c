#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include "funkcje.h"

int main(int argc, char **argv) {
	int sockFD, newSocket, portNO, addr_size;
	int reuse = 1;
	struct sockaddr_in serv_addr, newAddr;

	if (argc < 2) {
		fprintf(stderr, "Uruchomienie: sudo %s numer portu\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (sockFD < 0) {
		showError("Blad podczas towrzenia gniazda\n");
	}
	printf("Gniazdo poprawnie utworzone \n");

	memset((void*) &serv_addr, 0, sizeof(serv_addr));
	portNO = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portNO);

	if (bind(sockFD, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		showError("Blad podczas bindowania \n");
	}
	printf("Bindowanie wykonane poprawnie \n");

	setsockopt(sockFD, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

	if (listen(sockFD, 7) < 0) {
		showError("Blad podczas nasluchiwania serwera \n");
	}
	printf("Serwer rozpoczal nasluchiwanie... \n");

	addr_size = sizeof(newAddr);

	while (1) {
		memset(&newAddr, 0, sizeof(newAddr));
		if ((newSocket = accept(sockFD, (struct sockaddr*) &newAddr, &addr_size))
				< 0) {
			showError("Blad podczas akceptowania polaczenia \n");
		}
		switch (fork()) {
		case -1:
			printf("Blad podczas wywolywania procesu \n");
			break;
		case 0: {
			close(sockFD);
			connection(newSocket);
			exit(EXIT_SUCCESS);
			break;
		}
		default:
			close(newSocket);
			break;
		}
	}
	return 0;
}
