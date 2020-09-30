#include "loader.h"

extern unsigned char mem[]; // external array declared in memory.c
eptr es_table[HASH_SIZE]; // hash table for external symbols
int progaddr, csaddr, cslth, execaddr, prev_bp; // prev_bp : to restart the program from the previous breakpoint
int bp[MEM_SIZE]; // bp[i]=1 if breakpoint on i-th address is set
int reg_val[10]; // to store the value of the registers
typedef enum { A, X, L, B, S, T, F, None, PC, SW } reg_name; // to make indexing reg_val easier

void cmd_progaddr(char* address) { // when cmd is progaddr
    progaddr = hexa_str_to_int(address, strlen(address)-1);
    return;
}

int cmd_loader(char* filename1, char* filename2, char* filename3) { // when cmd is loader
    char *files[3] = {filename1, filename2, filename3}; // array of files
    memset(reg_val, 0, sizeof(reg_val)); // initialize the values of registers
    delete_es_table(); // initialize external symbol table
    if (!load_pass1(files)) return 0; // if error occurred from pass1
    if (!load_pass2(files)) return 0; // if error occurred from pass2
    return 1; // program succeed
}

void push_es_table(int hash, char* sym, int addr, int length) { // push a node to the hash table
    eptr tmp = (eptr)malloc(sizeof(ES_NODE)); // create new node
    strcpy(tmp->sym, sym);
    tmp->addr = addr;
    tmp->length = length;
    tmp->link = NULL;
    if (!es_table[hash]) es_table[hash] = tmp;
    else {
        eptr w;
        for (w = es_table[hash]; w->link; w = w->link); // go to the last node
        w->link = tmp; // push current node to the end of the linked list
    }
    return;
}

void delete_es_table() { // remove external symbol table
    for (int i = 0; i < HASH_SIZE; i++) {
        eptr next;
        for (eptr curr = es_table[i]; curr; curr = next) {
            next = curr->link;
            free(curr);
        }
        es_table[i] = NULL; // initializing
    }
    return;
}

int load_pass1(char* filenames[3]) { // pass1
    char line[LINE_SIZE];
    char sym[SYM_SIZE+1];
    char len[SYM_SIZE+1];
    char* tok;
    int hash;
    FILE* fp;

    printf("control symbol address length\nsection name\n");
    printf("-----------------------------\n");
    csaddr = progaddr;
    reg_val[PC] = progaddr; // intialize PC reg for run
    for (int file_num = 0; file_num < 3 && filenames[file_num]; file_num++) { // interate files
        if (!(fp = fopen(filenames[file_num], "r"))) {
            printf("File Doesn't Exist!\n"); // error occurred ; no file
            return 0;
        }
        while (fgets(line, LINE_SIZE, fp)) { // read a line from the file
            line[strlen(line)-1] = '\0';
            if (line[0] == 'E') break;
            else if (line[0] == 'H') { // H record
                strncpy(sym, &line[1], SYM_SIZE); // get program name
                tok = strtok(sym, DELIMITER); // remove whitespace
                cslth = hexa_str_to_int(&line[SYM_SIZE*2+1], SYM_SIZE-1); // get program size
                if ((hash = hash_function(tok)) == -1) {
                    printf("Invalid Control Section!\n"); // error occurred ; calculating hash value failed
                    return 0;
                }
                for (eptr w = es_table[hash]; w; w = w->link) { // search
                    if (!strcmp(w->sym, tok)) { // if found
                        printf("Duplicated External Symbol!\n"); // error occurred
                        return 0;
                    }
                }
                printf("%-6s           %04X   %04X\n", tok, csaddr, cslth);
                push_es_table(hash, tok, csaddr, cslth); // push it to the estab since it's valid 
            }
            else if (line[0] == 'D') { // D record
                for (int i = 1; i < strlen(line); i += SYM_SIZE*2) { // read one tuple of (symbol name, address) at a time
                    strncpy(sym, &line[i], SYM_SIZE); // get symbol name
                    tok = strtok(sym, DELIMITER); // remove whilespace
                    strncpy(len, &line[i+SYM_SIZE], SYM_SIZE); // get address
                    if ((hash = hash_function(tok)) == -1) {
                        printf("Invalid Control Section!\n"); // error occurred ; calculating hash value failed
                        return 0;
                    }
                    for (eptr w = es_table[hash]; w; w = w->link) { // search
                        if (!strcmp(w->sym, tok)) { // if found
                            printf("Duplicated External Symbol!\n"); // error occurred
                            return 0;
                        }
                    }
                    printf("         %-6s  %04X\n", tok, csaddr+hexa_str_to_int(len, SYM_SIZE-1));
                    push_es_table(hash, tok, csaddr+hexa_str_to_int(len, SYM_SIZE-1), -1); // push it to the estab since it's valid 
                }
            }
        }
        csaddr += cslth;
    }
    reg_val[L] = csaddr - progaddr; // L reg for run
    printf("-----------------------------\n");
    printf("           total length %04X\n", reg_val[L]);
    return 1;
}

int load_pass2(char* filenames[3]) { // pass2
    char line[LINE_SIZE];
    char sym[SYM_SIZE+1];
    char addr[SYM_SIZE+1];
    char* tok;
    int len, idx, n, x, y, hash;

    FILE* fp;

    csaddr = progaddr;
    execaddr = progaddr;
    for (int file_num = 0; file_num < 3 && filenames[file_num]; file_num++) {
        char ref_num[MAX_REF_NUM][SYM_SIZE+1]; // to store reference number of R record and M record
        if (!(fp = fopen(filenames[file_num], "r"))) {
            printf("File Doesn't Exist!\n"); // error occurred ; no file
            return 0;
        }
        while (fgets(line, LINE_SIZE, fp)) {
            line[strlen(line)-1] = '\0';
            if (line[0] == 'E') { // E record
                execaddr = hexa_str_to_int(&line[1], SYM_SIZE-1);
                break;
            }
            else if (line[0] == 'H') { // H record
                strncpy(sym, &line[1], SYM_SIZE); // get program name
                tok = strtok(sym, DELIMITER); // remove whitespace
                strcpy(ref_num[1], tok); // reference number of current program is 1
                cslth = hexa_str_to_int(&line[SYM_SIZE*2+1], SYM_SIZE-1);
            }
            else if (line[0] == 'R') {
                for (int i = 1; i < strlen(line); i += 8) { // read one tuple of (reference number, symbol name) at a time
                    idx = dec_str_to_int(&line[i], 1); // get reference number
                    strncpy(sym, &line[i+2], SYM_SIZE); // get symbol name
                    tok = strtok(sym, DELIMITER); // remove whilespace
                    strcpy(ref_num[idx], tok); // store the info
                }
            }
            else if (line[0] == 'T') { // T record
                strncpy(addr, &line[1], SYM_SIZE); // start address of current record
                idx = csaddr + hexa_str_to_int(addr, SYM_SIZE-1); // add control section addr to the addr
                for (int i = SYM_SIZE+3; i < strlen(line); i += 2)
                    mem[idx++] = hexa_str_to_int(&line[i], 1); // load the info to the memory 
            }
            else if (line[0] == 'M') { // M record
                strncpy(addr, &line[1], SYM_SIZE); // start address of current record
                idx = csaddr + hexa_str_to_int(addr, SYM_SIZE-1); // add control section addr to the addr
                len = hexa_str_to_int(&line[SYM_SIZE+1], 1); // how many half byte shold be modified
                n = dec_str_to_int(&line[SYM_SIZE+4], 1); // get reference number
                
                if ((hash = hash_function(ref_num[n])) == -1) {
                    printf("Invalid Control Section!\n");  // error occurred ; calculating hash value failed
                    return 0;
                }
                y = -1; // to store the addr of external symbol
                for (eptr w = es_table[hash]; w; w = w->link) { // search
                    if (!strcmp(w->sym, ref_num[n])) { // if found
                        y = w->addr;
                        break;
                    }
                }
                if (y == -1){ // not found
                    printf("Undefined External Symbol!\n"); // error occured
                    return 0;
                }
                x = 0; // to store existing value that will be modified
                for (int i = 0; i < len/2 + (len%2); i++) { // get the value
                    x *= 0x100;
                    if ((len%2) && !i) x += mem[idx+i] % 0x10;
                    else x += mem[idx+i];
                }
                if (line[9] == '+') x += y; // calculate modified value ; add
                else x -= y; // calculate modified value ; subtract
                unsigned int ux = x; // to represent negative numbers as 2'complement form
                for (int i = len/2 - 1 + (len%2); i >= 0; i--) { // load the modified value
                    if ((len%2) && !i) mem[idx+i] = (mem[idx+i]/0x10) * 0x10 + ux % 0x10;
                    else mem[idx+i] = ux % 0x100;
                    ux /= 0x100;
                }
            }
        }
        csaddr += cslth;
    }
    return 1;
}

void cmd_bp(char* option) { // when cmd is bp
    if (!option) { // no operand ; print existing break points
        printf("\tbreakpoint\n\t----------\n");
        for (int i = 0; i < MEM_SIZE; i++)
            if (bp[i]) printf("\t%0X\n", i);
    }
    else if (!strcmp(option, "clear")) { // clear
        memset(bp, 0, sizeof(bp));
        printf("\t[\x1b[32mok\x1b[0m] clear all breakpoints\n");
    }
    else { // set
        int idx = hexa_str_to_int(option, strlen(option)-1);
        bp[idx] = 1;
        printf("\t[\x1b[32mok\x1b[0m] create breakpoint %04X\n", idx);
    }
}

void cmd_run() { // when cmd is run
    reg_val[PC] = prev_bp;
    for (int i = prev_bp; (i < csaddr) && (i == prev_bp || bp[i] != 1); i = reg_val[PC]) {
        
        // format2
        reg_val[PC] += 2; // increase program counter
        if (mem[i] == 0xB4) { // CLEAR
            reg_val[mem[i+1]/0x10] = 0;
            continue;
        }
        if (mem[i] == 0xA0) { // COMPR
            reg_val[SW] = reg_val[mem[i+1]/0x10] - reg_val[mem[i+1]%0x10];
            continue;
        }
        if (mem[i] == 0xB8) { // TIXR
            reg_val[X]++;
            reg_val[SW] = reg_val[X] - reg_val[mem[i+1]/0x10];
            continue;
        }

        // format3,4
        int target, val = 0, e = (mem[i+1] & 16)/16, ni = mem[i] % 4;
        
        reg_val[PC] += 1 + e; // inc 3 if format3, inc 4 if format4

        if (e) target = (mem[i+1] % 0x10) * 0x10000 + mem[i+2] * 0x100 + mem[i+3]; // e ; format4
        else target = (mem[i+1] % 0x10) * 0x100 + mem[i+2]; // e ; format3

        if (mem[i+1] & 32) target = reg_val[PC] + (target & 0x800 ? ((target-1)^0xFFF)*(-1) : target); // p ; if negative use 2's complement
        if (mem[i+1] & 64) target += reg_val[B]; // b
        if (mem[i+1] & 128) target += reg_val[X]; // x
        
        // J instructions
        if (ni == 2) val = mem[target]*0x10000 + mem[target+1]*0x100 + mem[target+2]; // simple addressing
        else if (ni == 3) val = target; // immediate addressing
        if ((mem[i] - ni) == 0x3C) { // J
            reg_val[PC] = val;
            continue;
        }
        else if ((mem[i] - ni) == 0x30) { // JEQ
            if (!reg_val[SW]) {
                reg_val[PC] = val;
                continue;
            }
        }
        else if ((mem[i] - ni) == 0x38) { // JLT
            if (reg_val[SW] < 0) {
                reg_val[PC] = val;
                continue;
            }
        }
        else if ((mem[i] - ni) == 0x48) { // JSUB
            reg_val[L] = reg_val[PC];
            reg_val[PC] = val;
            continue;
        }

        // normal instructions
        if (ni == 1) val = target; // simple addressing
        else if (ni == 2) val = mem[mem[target]]; // indirect addressing
        else if (ni == 3) val = mem[target]; // simple addressing

        if ((mem[i] - ni) == 0x50) { // LDCH
            reg_val[A] = (reg_val[A]/0x100)*0x100;
            reg_val[A] += val; // no overflow?
        }
        else if ((mem[i] - ni) == 0xD8) { // RD
            reg_val[A] = (reg_val[A]/0x100)*0x100;
        }
        else if ((mem[i] - ni) == 0x4C) { // RSUB
            reg_val[PC] = reg_val[L];
            continue;
        }
        else if ((mem[i] - ni) == 0x0C) { // STA
            mem[target] = reg_val[A]/0x10000;
            mem[target+1] = (reg_val[A]%0x10000)/0x100;
            mem[target+2] = reg_val[A]%0x100;
        }
        else if ((mem[i] - ni) == 0x54) { // STCH
            mem[target] = reg_val[A]%0x100;
        }
        else if ((mem[i] - ni) == 0x14) { // STL
            mem[target] = reg_val[L]/0x10000;
            mem[target+1] = (reg_val[L]%0x10000)/0x100;
            mem[target+2] = reg_val[L]%0x100;
        }
        else if ((mem[i] - ni) == 0x10) { // STX
            mem[target] = reg_val[X]/0x10000;
            mem[target+1] = (reg_val[X]%0x10000)/0x100;
            mem[target+2] = reg_val[X]%0x100;
        }
        else if ((mem[i] - ni) == 0xE0) { // TD
            reg_val[SW] = -1;
        }
        
        // using 3 bytes value instructions
        if (ni == 2) val = mem[mem[target]] * 0x10000 + mem[mem[target]+1] * 0x100 + mem[mem[target]+2]; // indirect addressing
        else if (ni == 3) val = mem[target] * 0x10000 + mem[target+1] * 0x100 + mem[target+2]; // simple addressing

        if ((mem[i] - ni) == 0x28) { // COMP
            reg_val[SW] = reg_val[A] - val;
        }
        else if ((mem[i] - ni) == 0x00) { // LDA
            reg_val[A] = val;
        }
        else if ((mem[i] - ni) == 0x68) { // LDB
            reg_val[B] = val;
        }
        else if ((mem[i] - ni) == 0x74) { //LDT
            reg_val[T] = val;
        }
    }
    printf("A : %06X  X : %06X\n", reg_val[A], reg_val[X]);
    printf("L : %06X PC : %06X\n", reg_val[L], reg_val[PC]);
    printf("B : %06X  S : %06X\n", reg_val[B], reg_val[S]);
    printf("T : %06X\n", reg_val[T]);
    if (bp[reg_val[PC]]) {
        prev_bp = reg_val[PC]; // set prev_bp
        bp[reg_val[PC]] = -1; // to void eternal break at loop..
        printf("\t\tStop at checkpoint[%X]\n", reg_val[PC]);
    }
    else {
        prev_bp = progaddr; // reset prev_bp cause program has done
        printf("\t\tEnd Program\n");
    }
}
