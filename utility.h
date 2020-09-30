#ifndef UTIL
#define UTIL

#define DELIMITER " \t\r\n\f\v"
#define CMD_SIZE 81 // size of command string
#define LINE_SIZE 81 // length of a line
#define MEM_SIZE 16*65536 // size of memory = 1MB
#define MAX_MEM 0xFFFFF // maximum memory address
#define MAX_VAL 0xFF
#define SYM_SIZE 6
#define HASH_SIZE 20
#define OPCODE_LEN 3
#define MNEMONIC_LEN 11
#define LOC_SIZE 7
#define TREC_LEN 20
#define MAX_REF_NUM 50

int hexa_str_to_int(char*, int);
int dec_str_to_int(char*, int);

#endif
