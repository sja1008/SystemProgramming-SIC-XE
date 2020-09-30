#ifndef SHELL
#define SHELL

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include "utility.h"

typedef struct _HIST_NODE* hptr;
typedef struct _HIST_NODE { // node of the history linked list
    int val_cnt;
    char val_cmd[CMD_SIZE];
    hptr link;
} HIST_NODE;

void cmd_help();
void cmd_dir();
void push_history(char*);
void cmd_history();
int cmd_type(char*);

#endif
