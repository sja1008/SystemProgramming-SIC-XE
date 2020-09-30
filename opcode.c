#include "opcode.h"

optr hash_table[HASH_SIZE];
int hash_flag; // 1 if hash table has been constructed

int hash_function(char* key) { // hash function
    int hash = 0;
    for (int i = 0; i < strlen(key); i++) {
        if (key[i] < 'A' || key[i] > 'Z') return -1;
        hash += key[i]-'A';
    }
    return hash % HASH_SIZE;
}

void push_hash_table(int hash, char* opcode, char* mnemonic, int fmt) { // push a node to the hash table
    optr tmp = (optr)malloc(sizeof(OP_NODE)); // create new node
    strcpy(tmp->opcode, opcode);
    strcpy(tmp->mnemonic, mnemonic);
    tmp->fmt = fmt;
    tmp->link = NULL;
    if (!hash_table[hash]) hash_table[hash] = tmp;
    else {
        optr w;
        for (w = hash_table[hash]; w->link; w = w->link); // go to the last node
        w->link = tmp; // push current node to the end of the linked list
    }
    return;
}

int construct_hash_table() { // make and initialize the hash table
    char line[LINE_SIZE]; // a line in the file
    char* opcode;
    char* mnemonic;
    char* format;
    char* eol; // end of line : to check whether the line has appropriate number of columns
    int hash;
    FILE *fp = fopen("opcode.txt", "r"); // read file
    if (!fp) {
        printf("Opcode File Doesn't Exist!\n"); // error occurred ; no opcode file
        return 0;
    }
    while (fgets(line, LINE_SIZE, fp)) { // get a line in the file until eof
        line[strlen(line)-1] = '\0'; // change newline to null
        opcode = strtok(line, DELIMITER); // get opcode from the line if exist
        if (!opcode) break;
        mnemonic = strtok(NULL, DELIMITER); // get mnemonic from the line if exist
        format = strtok(NULL, DELIMITER); // get format from the line if exist
        eol = strtok(NULL, DELIMITER);
        if (eol || !format || !mnemonic) { // error occurred ; invalid number of columns
            printf("Invalid Opcode File!\n");
            fclose(fp);
            return 0;
        }
        if ((hash = hash_function(mnemonic))==-1) {
            printf("Invalid Opcode File!\n");
            return 0;
        }
        push_hash_table(hash, opcode, mnemonic, format[0]-'0');
    }
    fclose(fp);
    return 1; // hash_table has been constructed successfully 
}

int cmd_opcode(char* mnemonic) { // when cmd is opcode mnemonic
    if (!hash_flag) { // if hash table doesn't exist
        if (construct_hash_table()) hash_flag = 1; // construct it
        else return 0; // error occurred
    }
    int hash = hash_function(mnemonic);
    for (optr w = hash == -1 ? NULL : hash_table[hash]; w; w = w->link) { // search
        if (!strcmp(w->mnemonic, mnemonic)) { // if found
            printf("opcode is %s\n", w->opcode);
            return 1;
        }
    }
    printf("Invalid Mnemonic!\n"); // error occurred
    return 0; // error occurred
}

int cmd_opcodelist() { // when cmd is opcodelist
    if (!hash_flag) { // if hash table doesn't exist
        if (construct_hash_table()) hash_flag = 1; // construct it
        else return 0; // error occurred
    }
    for (int i = 0; i < HASH_SIZE; i++) { // print
        printf("%d : ", i);
        optr w = hash_table[i];
        if (!w) {
            printf("\n");
            continue;
        }
        printf("[%s,%s]", w->mnemonic, w->opcode);
        for (w = w->link; w; w = w->link)
            printf(" -> [%s,%s]", w->mnemonic, w->opcode);
        printf("\n");
    }
    return 1;
}
