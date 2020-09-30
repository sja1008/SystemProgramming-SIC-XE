#ifndef ASSEM
#define ASSEM

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"
#include "opcode.h"

#define CLOSE1 fclose(fp);fclose(tp);fclose(np);
#define CLOSE2 fclose(fp);fclose(tp);fclose(np);fclose(op);fclose(lp);

typedef struct _SYM_NODE* sptr;
typedef struct _SYM_NODE { // symbol table
	int addr;
	char sym[SYM_SIZE+1];
	sptr left, right; 
} SYM_NODE;

typedef struct _MREC_NODE* mptr;
typedef struct _MREC_NODE { // linked list node for mrecord 
	int mrec;
	mptr link;
} MREC_NODE;

int cmd_assemble(char*);
void cmd_symbol();
void init();
sptr search_pred(sptr, char*);
void insert(sptr*, int, char*);
sptr search(sptr, char*);
void inorder(sptr);
void destruct(sptr);
int is_mnemonic(char*);
int which_relative(int*, int, int);
int asm_pass1(char*);
int asm_pass2(char*);

#endif