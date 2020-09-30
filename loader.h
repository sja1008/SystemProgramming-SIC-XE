#ifndef LOAD
#define LOAD

#include <stdio.h>
#include <string.h>
#include "utility.h"
#include "opcode.h"

typedef struct _ES_NODE* eptr;
typedef struct _ES_NODE { // external symbol table
	char sym[SYM_SIZE+1];
    int addr;
    int length;
	eptr link;
} ES_NODE;

void cmd_progaddr(char*);
int cmd_loader(char*, char*, char*);
void push_es_table(int, char*, int, int);
void delete_es_table();
void loadmap();
int load_pass1(char*[3]);
int load_pass2(char*[3]);
void cmd_bp(char*);
void cmd_run();

#endif
