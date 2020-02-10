#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include "funkcje.h"

#define MAX_USERID_LEN 256
#define MAX_PASS_LEN 256
#define MAX_LEN 1024
#define MAX_LINES_IN_MS 20

#define USER 0
#define UNAUTH_USER -1

#define RESPONSE_BYTES 512
#define REQUEST_BYTES 512

struct userInfo {
	char userId[MAX_USERID_LEN + 1];
	char pass[MAX_PASS_LEN + 1];
};

void showError(char *str) {
	perror(str);
	exit(EXIT_FAILURE);
}

void wyswietl(char *str) {
	printf("%s", str);
}

void saveLog(char *str) {
	FILE *log = fopen("debug.log", "a");
	time_t now;
	time(&now);
	strcat(str, " ");
	strcat(str, ctime(&now));
	fprintf(log, str);
	fclose(log);
}

void sendMsgToClient(int newSocket, char *str) {
	int numPacketsToSend = (strlen(str) - 1) / RESPONSE_BYTES + 1;
	int n = write(newSocket, &numPacketsToSend, sizeof(int));
	char *msgToSend = (char*) malloc(numPacketsToSend * RESPONSE_BYTES);
	strcpy(msgToSend, str);
	int i;
	for (i = 0; i < numPacketsToSend; ++i) {
		int n = write(newSocket, msgToSend, RESPONSE_BYTES);
		msgToSend += RESPONSE_BYTES;
	}
}

char* receiveMsgFromClient(int newSocket) {
	int numPacketsToReceive = 0;
	int n = read(newSocket, &numPacketsToReceive, sizeof(int));
	if (n <= 0) {
		shutdown(newSocket, SHUT_WR);
		return NULL;
	}

	char *str = (char*) malloc(numPacketsToReceive * REQUEST_BYTES);
	memset(str, 0, numPacketsToReceive * REQUEST_BYTES);
	char *str_p = str;
	int i;
	for (i = 0; i < numPacketsToReceive; ++i) {
		int n = read(newSocket, str, REQUEST_BYTES);
		str = str + REQUEST_BYTES;
	}
	return str_p;
}

struct userInfo getUserInfo(int newSocket) {
	int n;
	char *username = "Login:";
	char *password = "Hasło:";
	char *buffU;
	char *buffP;

	sendMsgToClient(newSocket, username);
	buffU = receiveMsgFromClient(newSocket);

	sendMsgToClient(newSocket, password);
	buffP = receiveMsgFromClient(newSocket);

	struct userInfo uInfo;
	memset(&uInfo, 0, sizeof(uInfo));

	int i;
	for (i = 0; i < MAX_USERID_LEN; ++i) {
		if (buffU[i] != '\n' && buffU[i] != '\0') {
			uInfo.userId[i] = buffU[i];
		} else {
			break;
		}
	}
	uInfo.userId[i] = '\0';

	for (i = 0; i < MAX_PASS_LEN; ++i) {
		if (buffP[i] != '\n' && buffP[i] != '\0') {
			uInfo.pass[i] = buffP[i];
		} else {
			break;
		}
	}
	uInfo.pass[i] = '\0';
	if (buffU != NULL)
		free(buffU);
	buffU = NULL;
	if (buffP != NULL)
		free(buffP);
	buffP = NULL;
	return uInfo;
}

char* readFromFile(FILE *fp) {
	fseek(fp, 0, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (sz == 0)
		return NULL;
	char *str = (char *) malloc((sz + 1) * sizeof(char));
	fread(str, sizeof(char), sz, fp);
	str[sz] = 0;
	return str;
}

int authorizeUser(struct userInfo uInfo) {
	wyswietl("Sprawdzanie danych logowania: \n");
	wyswietl(uInfo.userId);
	wyswietl("\n");
	wyswietl(uInfo.pass);
	wyswietl("\n");

	FILE *fp = fopen("users_database", "r");
	char delim[] = ", \n";
	char *str = readFromFile(fp);
	fclose(fp);
	char *save_ptr;
	char *wyraz = strtok_r(str, delim, &save_ptr);
	do {
		if (strcmp(uInfo.userId, wyraz) == 0) {
			wyraz = NULL;
			wyraz = strtok_r(NULL, delim, &save_ptr);
			if (strcmp(uInfo.pass, wyraz) == 0) {
				wyraz = NULL;
				wyraz = strtok_r(NULL, delim, &save_ptr);
				return USER;
			}
		} else {
			wyraz = strtok_r(NULL, delim, &save_ptr);
			wyraz = strtok_r(NULL, delim, &save_ptr);
		}
		wyraz = NULL;
	} while ((wyraz = strtok_r(NULL, delim, &save_ptr)) != NULL);
	if (str != NULL)
		free(str);
	return UNAUTH_USER;
}

int validate(struct userInfo uInfo) {
	return authorizeUser(uInfo);
}

void closeWithMsg(char *str, int newSocket) {
	sendMsgToClient(newSocket, str);
	shutdown(newSocket, SHUT_RDWR);
}

void addStrings(char** str1, const char* str2, char del) {
	size_t len1 = *str1 ? strlen(*str1) : 0;
	size_t len2 = str2 ? strlen(str2) : 0;
	char *res = realloc(*str1, len1 + len2 + 2);
	if (res) {
		res[len1] = del;
		memcpy(res + len1 + 1, str2, len2);
		res[len1 + 1 + len2] = 0;
		*str1 = res;
	}
}

int sum(char* wyraz, char* amount) {
	int a = atoi(wyraz);
	int b = atoi(amount);
	return a + b;
}

int diff(char* wyraz, char* amount) {
	int a = atoi(wyraz);
	int b = atoi(amount);
	return a - b;
}

int ispossible(char* wyraz, char*amount) {
	int a = atoi(wyraz);
	int b = atoi(amount);
	if (a - b < 0) {
		return 0;
	} else {
		return 1;
	}
}

void inputBalance(int newSocket, struct userInfo uInfo) {
	char *command = "Podaj kwote do wplaty:";
	char *amount;
	char *final_word;
	char *saldo = "Stan konta: ";
	int final_amount;
	int length;
	sendMsgToClient(newSocket, command);
	amount = receiveMsgFromClient(newSocket);

	FILE *fp = fopen("users_database", "r+");
	char delim[] = ",\n";
	char *str = readFromFile(fp);
	fclose(fp);

	FILE *nf = fopen("temp", "a");
	char *balance = (char*) malloc(2 * sizeof(char));
	balance[0] = '0';
	balance[1] = 0;

	char *save_ptr;
	char *wyraz = strtok_r(str, delim, &save_ptr);
	do {
		fprintf(nf, wyraz);
		fprintf(nf, ",");
		if (strcmp(uInfo.userId, wyraz) == 0) { //user name matched
			wyraz = NULL;
			wyraz = strtok_r(NULL, delim, &save_ptr);
			fprintf(nf, wyraz);
			fprintf(nf, ",");
			if (strcmp(uInfo.pass, wyraz) == 0) { //password matched
				wyraz = NULL;
				wyraz = strtok_r(NULL, delim, &save_ptr);
				final_amount = sum(wyraz, amount);
				length = snprintf(NULL, 0, "%i", final_amount);
				wyraz = NULL;
				wyraz = (char*) malloc(length);
				sprintf(wyraz, "%d", final_amount);
				fprintf(nf, wyraz);
				fprintf(nf, "\n");
				final_word = (char *) malloc(1 + strlen(wyraz) + strlen(saldo));
				strcpy(final_word, saldo);
				strcat(final_word, wyraz);
				balance = (char *) malloc(1 + strlen(wyraz) + strlen(saldo));
				strcpy(balance, final_word);
				balance[strlen(final_word)] = 0;
				addStrings(&balance,
						"Co dalej?\n 1 - wplata,\n 2 - wyplata,\n 3 - stan konta,\n exit - wyjscie",
						'\n');
			}
		} else {
			wyraz = strtok_r(NULL, delim, &save_ptr);
			fprintf(nf, wyraz);
			fprintf(nf, ",");
			wyraz = strtok_r(NULL, delim, &save_ptr);
			fprintf(nf, wyraz);
			fprintf(nf, "\n");
		}
		wyraz = NULL;
	} while ((wyraz = strtok_r(NULL, delim, &save_ptr)) != NULL);
	fclose(nf);
	remove("users_database");
	rename("temp", "users_database");
	sendMsgToClient(newSocket, balance);
	if (balance != NULL)
		free(balance);
	balance = NULL;
	if (str != NULL)
		free(str);
	str = NULL;
	if (final_word != NULL)
		free(final_word);
	final_word = NULL;
}

void outputBalance(int newSocket, struct userInfo uInfo) {
	char *command = "Podaj kwote wyplaty:";
	char *amount;
	char *final_word;
	char *saldo = "Stan konta: ";
	sendMsgToClient(newSocket, command);
	amount = receiveMsgFromClient(newSocket);

	FILE *fp = fopen("users_database", "r+");
	char delim[] = ",\n";
	char *str = readFromFile(fp);
	fclose(fp);

	FILE *nf = fopen("temp", "a");
	char *balance = (char*) malloc(2 * sizeof(char));
	balance[0] = '0';
	balance[1] = 0;

	char *save_ptr;
	char *wyraz = strtok_r(str, delim, &save_ptr);
	do {
		fprintf(nf, wyraz);
		fprintf(nf, ",");
		if (strcmp(uInfo.userId, wyraz) == 0) { //user name matched
			wyraz = NULL;
			wyraz = strtok_r(NULL, delim, &save_ptr);
			fprintf(nf, wyraz);
			fprintf(nf, ",");
			if (strcmp(uInfo.pass, wyraz) == 0) { //password matched
				wyraz = NULL;
				wyraz = strtok_r(NULL, delim, &save_ptr);
				if (ispossible(wyraz, amount) == 1) {
					int final_amount = diff(wyraz, amount);
					sprintf(wyraz, "%d", final_amount);
					fprintf(nf, wyraz);
					fprintf(nf, "\n");
					final_word = (char *) malloc(
							1 + strlen(wyraz) + strlen(saldo));
					strcpy(final_word, saldo);
					strcat(final_word, wyraz);
					balance = (char *) malloc(
							1 + strlen(wyraz) + strlen(saldo));
					strcpy(balance, final_word);
					balance[strlen(final_word)] = 0;
					addStrings(&balance,
							"Co dalej?\n 1 - wplata,\n 2 - wyplata,\n 3 - stan konta,\n exit - wyjscie",
							'\n');
				} else {
					fprintf(nf, wyraz);
					fprintf(nf, "\n");
					final_word = (char *) malloc(
							1 + strlen(wyraz) + strlen(saldo));
					strcpy(final_word, saldo);
					strcat(final_word, wyraz);
					balance = (char *) malloc(
							1 + strlen(wyraz) + strlen(saldo));
					strcpy(balance, final_word);
					balance[strlen(final_word)] = 0;
					addStrings(&balance,
							"Nie posiadasz tylu srodkow na koncie, sproboj ponownie:\n1 - wplata,\n 2 - wyplata,\n 3 - stan konta,\n exit - wyjscie",
							'\n');
				}

			}
		} else {
			wyraz = strtok_r(NULL, delim, &save_ptr);
			fprintf(nf, wyraz);
			fprintf(nf, ",");
			wyraz = strtok_r(NULL, delim, &save_ptr);
			fprintf(nf, wyraz);
			fprintf(nf, "\n");
		}
		wyraz = NULL;
	} while ((wyraz = strtok_r(NULL, delim, &save_ptr)) != NULL);
	fclose(nf);
	remove("users_database");
	rename("temp", "users_database");
	sendMsgToClient(newSocket, balance);
	if (balance != NULL)
		free(balance);
	balance = NULL;
	if (str != NULL)
		free(str);
	str = NULL;
	if (final_word != NULL)
		free(final_word);
	final_word = NULL;
}

void returnBalance(int newSocket, struct userInfo uInfo) {
	FILE *fp = fopen("users_database", "r");
	char delim[] = ",\n";
	char *str = readFromFile(fp);
	fclose(fp);

	char* final_word;
	char *saldo = "Stan konta: ";

	char *balance = (char*) malloc(2 * sizeof(char));
	balance[0] = '0';
	balance[1] = 0;

	char *save_ptr;
	char *wyraz = strtok_r(str, delim, &save_ptr);
	do {
		if (strcmp(uInfo.userId, wyraz) == 0) { //user name matched
			wyraz = NULL;
			wyraz = strtok_r(NULL, delim, &save_ptr);
			if (strcmp(uInfo.pass, wyraz) == 0) { //password matched
				wyraz = NULL;
				wyraz = strtok_r(NULL, delim, &save_ptr);
				final_word = (char *) malloc(1 + strlen(wyraz) + strlen(saldo));
				strcpy(final_word, saldo);
				strcat(final_word, wyraz);
				balance = (char *) malloc(1 + strlen(wyraz) + strlen(saldo));
				strcpy(balance, final_word);
				balance[strlen(final_word)] = 0;
				addStrings(&balance,
						"Co dalej?\n 1 - wplata,\n 2 - wyplata,\n 3 - stan konta,\n exit - wyjscie",
						'\n');
			}
		} else {
			wyraz = strtok_r(NULL, delim, &save_ptr);
			wyraz = strtok_r(NULL, delim, &save_ptr);
		}
		wyraz = NULL;
	} while ((wyraz = strtok_r(NULL, delim, &save_ptr)) != NULL);
	sendMsgToClient(newSocket, balance);
	if (balance != NULL)
		free(balance);
	balance = NULL;
	if (str != NULL)
		free(str);
	str = NULL;
	if (final_word != NULL)
		free(final_word);
	final_word = NULL;
}

void processUserRequests(int newSocket, struct userInfo uInfo) {
	char *buff = NULL;
	sendMsgToClient(newSocket,
			"Co chcesz zrobić?\n 1 - wplata,\n 2 - wyplata,\n 3 - stan konta,\n exit - wyjscie");
	while (1) {
		if (buff != NULL)
			free(buff);
		buff = receiveMsgFromClient(newSocket);
		if (strcmp(buff, "1") == 0) {
			inputBalance(newSocket, uInfo);
		} else if (strcmp(buff, "2") == 0) {
			outputBalance(newSocket, uInfo);
		} else if (strcmp(buff, "3") == 0) {
			returnBalance(newSocket, uInfo);
		} else if (strcmp(buff, "exit") == 0) {
			break;
		} else {
			sendMsgToClient(newSocket, "Niepoprawne polecenie.");
		}
	}
	if (buff != NULL)
		free(buff);
	buff = NULL;
}

void processRequests(int uType, int newSocket, struct userInfo uInfo) {
	char *blogin = "Bledne logowanie ";
	char *clogin = "Poprawne logowanie ";
	char *message;
	if (uType == UNAUTH_USER) {
		message = (char *) malloc(strlen(blogin) + strlen(uInfo.userId) + 1);
		strcpy(message, blogin);
		strcat(message, uInfo.userId);
		saveLog(message);
		free(message);
		message = NULL;
		wyswietl("Nieupowazniony uzytkownik.\n");
		closeWithMsg("unauth", newSocket);
	} else if (uType == USER) {
		message = (char *) malloc(strlen(clogin) + strlen(uInfo.userId) + 1);
		strcpy(message, clogin);
		strcat(message, uInfo.userId);
		saveLog(message);
		free(message);
		message = NULL;
		wyswietl("Poprawnie zalogowano.\n");
		processUserRequests(newSocket, uInfo);
		closeWithMsg("Dzieki za wizyte!", newSocket);
	}
}

void connection(int newSocket) {
	struct userInfo uInfo = getUserInfo(newSocket);
	int uType = validate(uInfo);
	processRequests(uType, newSocket, uInfo);
}
