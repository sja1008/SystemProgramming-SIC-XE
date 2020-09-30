#ifndef OPCODE
#define OPCODE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

typedef struct _OP_NODE* optr;
typedef struct _OP_NODE { // node of the hash linked list
    char opcode[OPCODE_LEN];
    char mnemonic[MNEMONIC_LEN];
    int fmt;
    optr link;
} OP_NODE;

int hash_function(char*);
void push_hash_table(int, char*, char*, int);
int construct_hash_table();
int cmd_opcode(char*);
int cmd_opcodelist();

#endif
