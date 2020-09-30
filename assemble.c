#include "assemble.h"

extern int hash_flag; // external variable declared in opcode.c
extern optr hash_table[]; // external array declared in opcode.c
sptr symtab; // recently complited symbol table
sptr curr; // current symbol table
sptr reg; // register symbol table

int cmd_assemble(char* filename) { // when cmd is assemble filename
    if (!hash_flag) { // if hash table doesn't exist
        if (construct_hash_table()) hash_flag = 1; // construct it
        else return 0; // error occurred
    }
    init();
    if (!asm_pass1(filename)) { // assemble error occurred
        remove("temp.txt"); // remove immediate files
        remove("newline.txt");
        return 0;
    }
    int ret;
    if ((ret = asm_pass2(filename))) { // assemble succeed
        if (symtab) destruct(symtab); // destruct previous symtab
        symtab = curr; // store new symtab
        curr = NULL; // reset current symtab
    }
    remove("temp.txt"); // remove immediate files
    remove("newline.txt");
    return ret;
}

void cmd_symbol() { // when cmd is symbol
    inorder(symtab); // inorder BST
    return;
}

void init() { // preprocessing before assembling
    destruct(curr); // in case that previous assembling has failed with remaining symtab
    curr = NULL;
    if (reg) return; // if register has been stored as symbol
    insert(&reg, 0, "A"); // else
    insert(&reg, 1, "X");
    insert(&reg, 2, "L");
    insert(&reg, 3, "B");
    insert(&reg, 4, "S");
    insert(&reg, 5, "T");
    insert(&reg, 6, "F");
    insert(&reg, 8, "PC");
    insert(&reg, 9, "SW");
    return;
}

sptr search_pred(sptr root, char* key) { // looking for predecessor to insert new node
    sptr parent = NULL; // predecessor
	while (root) {
		if (!strcmp(key, root->sym)) return NULL; // BST don't have duplicated node
		if (strcmp(key, root->sym) < 0) { // go left
			parent = root;
			root = root->left;
		}
		else { // go right
			parent = root;
			root = root->right;
		}
	}
	return parent;
}

void insert(sptr* root, int addr, char* sym) { // insert new node to BST
    sptr succ, pred = search_pred(*root, sym); // successor, predecessor
    if (pred || !*root) { // if predecessor found || empty BST
		succ = (sptr)malloc(sizeof(SYM_NODE));
		succ->addr = addr;
		strcpy(succ->sym, sym);
		succ->left = succ->right = NULL;
		if (*root) { // if predecessor found
            if (strcmp(sym, pred->sym) < 0) //successor is left child of pred
				pred->left = succ; 
			else pred->right = succ; // successor is right child of pred
		}
		else *root = succ; // empty BST
	}
    return;
}

sptr search(sptr root, char* key) { // looking for the node that has the key
	if (!root) return NULL; // not found
    if (!strcmp(key, root->sym)) return root; // found
	if (strcmp(key, root->sym) < 0) // go left
		return search(root->left, key);
	return search(root->right, key); // go right
}

void inorder(sptr root) { // to get sorted result
    if (!root) return;
	inorder(root->left);
	printf("        %-8s %04X\n", root->sym, root->addr);
	inorder(root->right);
}

void destruct(sptr root) { // destruct BST via postorder
	if (!root) return;
    destruct(root->left);
    destruct(root->right);
	free(root);
}

int is_mnemonic(char* tok) { // to know whether tok is in optable or not
    int e_flag = 0; // extended mode
    if (tok[0] == '+') {
        e_flag = 1;
        tok = &tok[1];
    }
    int hash = hash_function(tok); // hash of token
    for (optr w = hash == -1 ? NULL : hash_table[hash]; w; w = w->link) { // search optable
        if (!strcmp(w->mnemonic, tok)) { // if found
            return e_flag + w->fmt; // if mnemonic return format
        }
    }
    return 0; // not found
}

int which_relative(int *disp, int pc, int base) { // to know which relative is appropriate
    if (*disp-pc >= -2048 && *disp-pc <= 2047) { // pc relative
        *disp -= pc; // update disp
        *disp &= 0xfff; // to get appropriate negative number
        return 2;
    }
    else if (base != -1 && *disp-base >= 0 && *disp-base <= 4095) { // base relative
        *disp -= base; // update disp
        return 4;
    }
    else return 0;
}

int asm_pass1(char* filename) {
    int start = 0, loc = 0, format, end_flag=0; // start addr, locctr, fomat of mnemonic, 1 if END exist
    int newline_loc, prev_loc = 0,  prev_res_flag = 1, curr_res_flag; // start loc of a text record line for obj file, previous locctr, 1 if RESB or RESW previously, 1 if RESB or RESW currently
    char *label, *mnemonic, *operand;
    char line[LINE_SIZE]; // store a line of asm file

    FILE *fp = fopen(filename, "r"); // read file
    FILE *tp = fopen("temp.txt", "w"); // intermediate file : store locctr of each line of asm file
    FILE *np = fopen("newline.txt", "w"); // intermediate file : store start and end locctr of a text record line for obj file

    if (!fp) {
        printf("File Doesn't Exist!\n"); // error occurred ; no file
        return 0;
    }

    fprintf(tp, "         \n%04X\n",start);

    for (int line_num = 5; fgets(line, LINE_SIZE, fp); line_num += 5) { // read a line of asm file
        if ((line[0] >= '0' && line[0] <= '9') || (line[0] >= 'A' && line[0] <= 'Z')) { // if label exists
            label = strtok(line, DELIMITER); // get label
            mnemonic = strtok(NULL, DELIMITER);
        }
        else {
            label = NULL; // no label
            mnemonic = strtok(line, DELIMITER); // get mnemonic
        }
        operand = strtok(NULL, DELIMITER);
    
        if (!mnemonic || mnemonic[0] == '.') continue; // empty line || comment line
        if (!strcmp(mnemonic, "BASE") || !strcmp(mnemonic, "NOBASE")) continue; // BASE || NOBASE
        if (!strcmp(mnemonic, "END")) {
            end_flag = 1; // set end flag since END exists
            break;
        }
        if (!strcmp(mnemonic, "START")) { // when START exists
            prev_loc = start = loc = hexa_str_to_int(operand, strlen(operand)-1); // get start addr
            fseek(tp, -5, SEEK_CUR); // move file pointer and..
            fprintf(tp, "%04X\n", start); // change start addr
            continue;
        }

        if (label) { // if label exists
            if (is_mnemonic(label) ||
            !strcmp(label, "BYTE") || !strcmp(label, "WORD") ||
            !strcmp(label, "RESB") || !strcmp(label, "RESW")) {
                printf("Line %d : Reserved Word Can't be a Label!\n", line_num); // error occurred
                CLOSE1
                return 0;
            }
            if (search(curr, label)) {
                printf("Line %d : Duplicate Symbol!\n", line_num); // error occurred
                CLOSE1
                return 0;
            }
            insert(&curr, loc, label);
        }

        curr_res_flag = 0;

        if (!strcmp(mnemonic, "BYTE")) {
            if (operand[0] == 'C')
                for (int i = 2; operand[i]!='\''; i++) loc += 1;
            else if (operand[0] == 'X')
                for (int i = 2; operand[i]!='\''; i+=2) loc += 1;
            else {
                printf("Line %d : Invaild Operand!\n", line_num);
                CLOSE1
                return 0;
            }
        }

        else if (!strcmp(mnemonic, "WORD"))
            loc += 3;

        else if (!strcmp(mnemonic, "RESB")) {
            loc += dec_str_to_int(operand, strlen(operand)-1);
            curr_res_flag = 1;
        }

        else if (!strcmp(mnemonic, "RESW")) {
            loc += 3 * dec_str_to_int(operand, strlen(operand)-1);
            curr_res_flag = 1;
        }

        else if (!(format = is_mnemonic(mnemonic))) { // mnemonic not found
            printf("Line %d : Invalid Operation Code!\n", line_num); // error occurred
            CLOSE1
            return 0;
        }

        else loc += format;

        if (!prev_res_flag && curr_res_flag) fprintf(np, "%04X\n",  prev_loc); // finish the text record line since RESB or RESW appear
        else if (!prev_res_flag && loc - newline_loc > 30) fprintf(np, "%04X\n%04X ", newline_loc = prev_loc, prev_loc); // change text record line since too long
        if (prev_res_flag && !curr_res_flag) fprintf(np, "%04X ", newline_loc = prev_loc); // start new text record line since switched RES to not

        fprintf(tp, "%04X\n", loc);
        prev_loc = loc;
        prev_res_flag = curr_res_flag;
    }
    rewind(tp); // move tp to the begining of temp.txt
    fprintf(tp, "%04X %04X\n", start, loc-start); // write start addr and program size
    fprintf(np, "%04X\n", loc); // write last addr
    CLOSE1
    if (!end_flag) { // no END
        printf("No END Line!\n");
        return 0;
    }
    return 1; // succeed
}

int asm_pass2(char* filename) {
    int loc, size, format, obj_start, obj_end = -1; // obj_start : start addr of a text record, obj_end : end addr of a text record
    int opcode, base = -1, ni, x, bp, e, disp; // base is -1 if NOBASE
    int error_flag = 0; // 1 if error
    char *label, *mnemonic, *operand1, *operand2;
    char line[LINE_SIZE];
    char line_cpy[LINE_SIZE];
    sptr tmp;
    mptr mrecord = NULL;

    FILE *fp = fopen(filename, "r"); // read file ; always exists since it confirmed by pass1
    FILE *tp = fopen("temp.txt", "r"); // read intermediate file
    FILE *np = fopen("newline.txt", "r"); // read intermediate file
    strcpy(&filename[strlen(filename)-3], "lst"); // change .asm extension to .lst
    FILE *lp = fopen(filename, "w"); // write lst file
    strcpy(&filename[strlen(filename)-3], "obj"); // change .lst extension to .obj
    FILE *op = fopen(filename, "w"); // write obj file

    fscanf(tp, "%X%X", &loc, &size); // start addr, program size
    fprintf(op, "H      %06X%06X\n", loc, size);
    for (int line_num = 5; fgets(line, LINE_SIZE, fp); line_num += 5) {
        strcpy(line_cpy, line);
        line[strlen(line)-1] = '\0';

        if ((line[0] >= '0' && line[0] <= '9') || (line[0] >= 'A' && line[0] <= 'Z')) { // if label exists
            label = strtok(line_cpy, DELIMITER);
            mnemonic = strtok(NULL, DELIMITER);
        }
        else { // no label
            label = NULL;
            mnemonic = strtok(line_cpy, DELIMITER);
        }
        operand1 = strtok(NULL, DELIMITER);
        operand2 = strtok(NULL, DELIMITER);

        fprintf(lp, "%-8d", line_num);
        if (!mnemonic || mnemonic[0] == '.') { // empty line || comment line
            fprintf(lp, "        %s\n", line);
            continue;
        }
        
        if (operand1 && operand2) operand1[strlen(operand1)-1] = '\0'; // remove ',' betweem op1 and op2
        e = mnemonic[0] == '+' ? 1 : 0; // extended
        x = (operand2 && operand2[0]=='X') ? 1 : 0; // indexing

        if (!strcmp(mnemonic, "NOBASE")) {
            fprintf(lp, "        %s\n", line);
            continue;
        }

        if (!strcmp(mnemonic, "BASE")) {
            fprintf(lp, "        %s\n", line);
            if ((tmp = search(curr, operand1))) base = tmp->addr; // operand is label
            else base = hexa_str_to_int(operand1, strlen(operand1)-1);
            continue;
        }

        if (!strcmp(mnemonic, "END")) {
            for (mptr w = mrecord; w; w = w->link) // print M record
                fprintf(op, "\nM%06X05", w->mrec);
            if (operand1) fprintf(op, "\nE%06X\n", search(curr, operand1)->addr);
            else fprintf(op, "\nE\n");
            fprintf(lp, "        %s\n", line);
            break;
        }

        if (!strcmp(mnemonic, "START")) { // when START exists
            fseek(op, -19 ,SEEK_CUR);
            fprintf(op, "%-6s", label); // fill program name
            fseek(op, 13 ,SEEK_CUR);
            fprintf(lp, "%04X    ", loc);
            fprintf(lp, "%s\n", line);
            continue;
        }

        fscanf(tp, "%X", &loc);
        if (loc >= obj_end) { // new T record line
            if (obj_end != -1) fprintf(op, "\n"); // if not the first line
            fscanf(np, "%04X %04X", &obj_start, &obj_end);
            fprintf(op, "T%06X%02X", obj_start, obj_end-obj_start);
        }
        fprintf(lp, "%04X    ", loc);
        fprintf(lp, "%-28s", line);

        if (!strcmp(mnemonic, "BYTE") || !strcmp(mnemonic, "WORD")) {
            if (mnemonic[0] != 'B') { // not BYTE
                fprintf(op, "%06X", dec_str_to_int(operand1, strlen(operand1)-1));
                fprintf(lp, "%06X", dec_str_to_int(operand1, strlen(operand1)-1));
                fprintf(lp, "\n");
                continue;
            }
            if (operand1[0] == 'C') { // BYTE C''
                for (int i = 2; operand1[i]!='\''; i += 1) {
                    fprintf(op, "%X", operand1[i]);
                    fprintf(lp, "%X", operand1[i]);
                }
                fprintf(lp, "\n");
                continue;
            }
            for (int i = 2; operand1[i]!='\''; i += 1) { // BYTE X''
                fprintf(op, "%C", operand1[i]);
                fprintf(lp, "%C", operand1[i]);
            }
            fprintf(lp, "\n");
            continue;
        }

        if (!strcmp(mnemonic, "RESB") || !strcmp(mnemonic, "RESW")) {
            fprintf(lp, "\n");
            continue;
        }

        int hash = hash_function(&mnemonic[e]);
        for (optr w = hash_table[hash]; w; w = w->link) { // search
            if (!strcmp(w->mnemonic, &mnemonic[e])) { // found ; always found since already confirmed in pass1
                format = w->fmt;
                opcode = hexa_str_to_int(w->opcode, strlen(w->opcode)-1);
                break;
            }
        }

        if (format == 1) { // format1
            fprintf(op, "%02X", opcode);
            fprintf(lp, "%02X\n", opcode);
        }

        else if (format == 2) { // format2
            fprintf(op, "%02X", opcode);
            fprintf(lp, "%02X", opcode);
            sptr r;
            fprintf(op, "%d", !operand1 ? 0 : (r = search(reg, operand1)) ? r->addr : (error_flag = 1)); // delay error checking for the readability
            fprintf(lp, "%d", !operand1 ? 0 : r ? r->addr : 0); // the last 0 is meaningless
            fprintf(op, "%d", !operand2 ? 0 : (r = search(reg, operand1)) ? r->addr : !strncmp(mnemonic, "SHIFT", 5) ? dec_str_to_int(operand2, strlen(operand2)-1) : (error_flag = 1));
            fprintf(lp, "%d\n", !operand2 ? 0 : r ? r->addr : !strncmp(mnemonic, "SHIFT", 5) ? dec_str_to_int(operand2, strlen(operand2)-1) : 0);
            if (error_flag) {
                printf("Line %d : Invalid Operand!\n", line_num); // error occurred
                CLOSE2
                remove(filename); // remove .obj
                strcpy(&filename[strlen(filename)-3], "lst"); // change .obj extension to .lst
                remove(filename); // remove .lst
                return 0;
            }
        }

        else if (mnemonic[0] != '+') { // format3
            if (!strcmp(mnemonic, "RSUB")) {
                if (operand1) {
                    printf("Line %d : Too Many Operands!\n", line_num); // error occurred
                    CLOSE2
                    remove(filename); // remove .obj
                    strcpy(&filename[strlen(filename)-3], "lst"); // change .obj extension to .lst
                    remove(filename); // remove .lst
                    return 0;
                }
                fprintf(op, "%02X0000", opcode+3);
                fprintf(lp, "%02X0000\n", opcode+3);
                continue;
            }
            ni = operand1[0] == '#' ? 1 : operand1[0] == '@' ? 2 : 3;
            if ((tmp = search(curr, &operand1[ni!=3]))) disp = tmp->addr; // label
            else if (ni==1) disp = dec_str_to_int(&operand1[ni!=3], strlen(operand1)-1-(ni!=3)); // not label
            else disp = hexa_str_to_int(&operand1[ni!=3], strlen(operand1)-1-(ni!=3));
            if (disp == -1) {
                printf("Line %d : Invalid Operand!\n", line_num); // error occurred
                CLOSE2
                remove(filename); // remove .obj
                strcpy(&filename[strlen(filename)-3], "lst"); // change .obj extension to .lst
                remove(filename); // remove .lst
                return 0;
            }
            bp = tmp ? which_relative(&disp, loc+3, base) : 0; // if label then look for proper relative and compute disp
            if (tmp && !bp) {
                printf("Line %d : Invalid Address!\n", line_num); // error occurred
                CLOSE2
                remove(filename); // remove .obj
                strcpy(&filename[strlen(filename)-3], "lst"); // change .obj extension to .lst
                remove(filename); // remove .lst
                return 0;
            }
            fprintf(op, "%02X%01X%03X", opcode+ni, 8*x+bp+e, disp);
            fprintf(lp, "%02X%01X%03X\n", opcode+ni, 8*x+bp+e, disp);
            if (error_flag) {
                printf("Line %d : Invalid Operand!\n", line_num); // error occurred
                CLOSE2
                remove(filename); // remove .obj
                strcpy(&filename[strlen(filename)-3], "lst"); // change .obj extension to .lst
                remove(filename); // remove .lst
                return 0;
            }
        }
        
        else { // format4 ; similar to format3
            if (!strcmp(mnemonic, "RSUB")) {
                if (operand1){
                    printf("Line %d : Too Many Operands!\n", line_num); // error occurred
                    CLOSE2
                    remove(filename); // remove .obj
                    strcpy(&filename[strlen(filename)-3], "lst"); // change .obj extension to .lst
                    remove(filename); // remove .lst
                    return 0;
                }
                fprintf(op, "%02X000000", opcode+3);
                fprintf(lp, "%02X000000\n", opcode+3);
                continue;
            }
            ni = operand1[0] == '#' ? 1 : operand1[0] == '@' ? 2 : 3;
            if ((tmp = search(curr, &operand1[ni!=3]))) { // store M record
                disp = tmp->addr;
                mptr new_mrec = (mptr)malloc(sizeof(MREC_NODE));
                new_mrec->mrec = loc+1;
                new_mrec->link = NULL;
                if (!mrecord) mrecord = new_mrec;
                else {
                    for (mptr w = mrecord; ; w = w->link) {
                        if (!w->link) {
                            w->link = new_mrec;
                            break;
                        }
                    }
                }
            }
            else if (ni==1) disp = dec_str_to_int(&operand1[ni!=3], strlen(operand1)-1-(ni!=3));
            else disp = hexa_str_to_int(&operand1[ni!=3], strlen(operand1)-1-(ni!=3));
            if (disp == -1) {
                printf("Line %d : Invalid Operand!\n", line_num); // error occurred
                CLOSE2
                remove(filename); // remove .obj
                strcpy(&filename[strlen(filename)-3], "lst"); // change .obj extension to .lst
                remove(filename); // remove .lst
                return 0;
            }
            bp = 0;
            fprintf(op, "%02X%01X%05X", opcode+ni, 8*x+bp+e, disp);
            fprintf(lp, "%02X%01X%05X\n", opcode+ni, 8*x+bp+e, disp);
        }
    }
    CLOSE2
    return 1;
}
