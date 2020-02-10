#ifndef _funkcje_H_
#define _funkcje_H_
#include "funkcje.c"

void showError(char *str);

void wyswietl(char *str);

void loguj(char *str);

void sendMsgToClient(int clientFD, char *str);

char* receiveMsgFromClient(int clientFD);

struct userInfo getUserInfo(int clientFD);

char* readFromFile(FILE *fp);

int authorizeUser(struct userInfo uInfo);

int validate(struct userInfo uInfo);

void closeWithMsg(char *str, int clientFD);

void addStrings(char** str1, const char* str2,char del);

int sum(char* wyraz, char* amount);

int diff(char* wyraz, char* amount);

int ispossible(char* wyraz, char*amount);

void inputBalance(int clientFD, struct userInfo uInfo);

void outputBalance(int clientFD, struct userInfo uInfo);

void returnBalance(int clientFD, struct userInfo uInfo);

void processUserRequests(int clientFD, struct userInfo uInfo);

void processRequests(int uType, int clientFD, struct userInfo uInfo);

void connection(int clientFD);

#endif _funkcje_H_
