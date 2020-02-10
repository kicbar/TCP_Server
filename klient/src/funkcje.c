#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include "funkcje.h"

#define RESPONSE_BYTES 512
#define REQUEST_BYTES 512

void showError(char *str) {
	perror(str);
	exit(EXIT_FAILURE);
}

void wyswietl(char *str) {
	printf("%s", str);
}

char* receiveMsgFromServer(int sockFD) {
	int numPacketsToReceive = 0;
	int n = read(sockFD, &numPacketsToReceive, sizeof(int));
	if (n <= 0) {
		shutdown(sockFD, SHUT_WR);
		return NULL;
	}
	char *str = (char*) malloc(numPacketsToReceive * RESPONSE_BYTES);
	memset(str, 0, numPacketsToReceive * RESPONSE_BYTES);
	char *str_p = str;
	int i;
	for (i = 0; i < numPacketsToReceive; ++i) {
		int n = read(sockFD, str, RESPONSE_BYTES);
		str = str + RESPONSE_BYTES;
	}
	return str_p;
}

void sendMsgToServer(int sockFD, char *str) {
	int numPacketsToSend = (strlen(str) - 1) / REQUEST_BYTES + 1;
	int n = write(sockFD, &numPacketsToSend, sizeof(int));
	char *msgToSend = (char*) malloc(numPacketsToSend * REQUEST_BYTES);
	strcpy(msgToSend, str);
	int i;
	for (i = 0; i < numPacketsToSend; ++i) {
		int n = write(sockFD, msgToSend, REQUEST_BYTES);
		msgToSend += REQUEST_BYTES;
	}
}
