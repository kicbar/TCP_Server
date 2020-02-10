#ifndef _funkcje_H_
#define _funkcje_H_
#include "funkcje.c"

void showError(char *str);

void wyswietl(char *str);

char* receiveMsgFromServer(int sockFD);

void sendMsgToServer(int sockFD, char *str);

#endif _funkcje_H_
