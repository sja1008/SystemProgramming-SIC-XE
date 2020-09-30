#include "20160563.h"

int main() {
    char cmd[CMD_SIZE]; // user command
    char cmd_cpy[CMD_SIZE]; // copied user command
    int dump_cnt = -1; // last printed address

    while(1) {
        printf("sicsim> ");
        fgets(cmd, CMD_SIZE, stdin); // get user command from stdin
        
        cmd[strlen(cmd)-1] = '\0'; // change newline to null
        strcpy(cmd_cpy, cmd); //copy user command cause strtok change the string
        
        char *cmd_ptr = strtok(cmd_cpy, DELIMITER); // get splitted string by space
        if (!cmd_ptr) continue; // user didn't give any cmd
        char *eos; // end of string : to check whether cmd has appropriate number of arguments

        if (!strcmp(cmd_ptr,"h") || !strcmp(cmd_ptr,"help")) { // when cmd is h[elp]
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            cmd_help();
            push_history(cmd); // since cmd is valid, push it to the history
        }

        else if (!strcmp(cmd_ptr,"d") || !strcmp(cmd_ptr,"dir")) { // when cmd is d[ir]
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            cmd_dir();
            push_history(cmd); // since cmd is valid, push it to the history
        }
        
        else if (!strcmp(cmd_ptr,"q") || !strcmp(cmd_ptr,"quit")) { // when cmd is q[uit]
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            break; // stop getting user command
        }

        else if (!strcmp(cmd_ptr,"hi") || !strcmp(cmd_ptr,"history")) { // when cmd is hi[story]
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            push_history(cmd); // since cmd is valid, push it to the history
            cmd_history();
        }

        else if (!strcmp(cmd_ptr, "du") || !strcmp(cmd_ptr, "dump")) { // when cmd is du[mp] [start [end]]
            char* start = strtok(NULL, DELIMITER); // get start addr if exist
            char* end = strtok(NULL, DELIMITER); // get end addr if exist
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            int start_num = 0, end_num = 0; // store integer value of address

            if (!start) { // when cmd is du[mp]
                push_history(cmd); // since cmd is valid, push it to the history
                cmd_dump(dump_cnt+1, (dump_cnt+160 < MAX_MEM ? dump_cnt+160 : MAX_MEM)); // boundary managing
                dump_cnt = dump_cnt+160 < MAX_MEM ? dump_cnt+160 : -1; // boundary managing
                continue;
            }

            if (end && start[strlen(start)-1] != ',') { // error occurred ; there's no ',' between start and end
                printf("Invalid Arguments!\n");
                continue;
            }

            start_num = hexa_str_to_int(start, end ? strlen(start)-2 : strlen(start)-1); // remove ',' from start string if end string exist
            if (start_num > MAX_MEM || start_num == -1) { // error occurred ; invalid boundary || invalid hexadecimal number
                printf("Invalid Arguments!\n");
                continue;
            }

            if (!end) { // when cmd is du[mp] start
                push_history(cmd); // since cmd is valid, push it to the history
                cmd_dump(start_num, (start_num+159 < MAX_MEM ? start_num+159 : MAX_MEM)); // boundary managing
                dump_cnt = start_num+159 < MAX_MEM ? start_num+159 : -1; // boundary managing
                continue;
            }

            end_num = hexa_str_to_int(end, strlen(end)-1);
            if (end_num > MAX_MEM || end_num < start_num || end_num == -1) { // error occurred ; invalid boundary || start is bigger then end || invalid hexadecimal number
                printf("Invalid Arguments!\n");
                continue;
            }
  
            else { // when cmd is du[mp] start end
                push_history(cmd); // since cmd is valid, push it to the history
                cmd_dump(start_num, end_num);
                dump_cnt = end_num != MAX_MEM ? end_num : -1; // boundary managing
            }
        }

        else if (!strcmp(cmd_ptr, "e") || !strcmp(cmd_ptr, "edit")) { // when cmd is e[dit]
            char* address = strtok(NULL, DELIMITER);
            char* value = strtok(NULL, DELIMITER);
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            int address_num = 0, value_num = 0;

            if (!address || address[strlen(address)-1] != ',' || !value) { // error occurred : missing required argument || no ','
                printf("Invalid Arguments!\n");
                continue;
            } 

            address_num = hexa_str_to_int(address, strlen(address)-2);
            value_num = hexa_str_to_int(value, strlen(value)-1);
            if (address_num > MAX_MEM || address_num == -1 || value_num > MAX_VAL || value_num == -1) { // error occurred ; invalid boundary || invalid hexadecimal number
                printf("Invalid Arguments!\n");
                continue;
            }

            push_history(cmd); // since cmd is valid, push it to the history
            cmd_edit(address_num, value_num);
        }

        else if (!strcmp(cmd_ptr, "f") || !strcmp(cmd_ptr, "fill")) { // when cmd is f[ill]
            char* start = strtok(NULL, DELIMITER);
            char* end = strtok(NULL, DELIMITER);
            char* value = strtok(NULL, DELIMITER);
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            int start_num = 0, end_num = 0, value_num = 0;

            if (!start || start[strlen(start)-1] != ',' || !end || end[strlen(end)-1] != ',' || !value) { // error occurred : missing required argument || no ','
                printf("Invalid Arguments!\n");
                continue;
            }

            start_num = hexa_str_to_int(start, strlen(start)-2);
            end_num = hexa_str_to_int(end, strlen(end)-2);
            value_num = hexa_str_to_int(value, strlen(value)-1);
            if (start_num > MAX_MEM || start_num == -1 
            || end_num > MAX_MEM || end_num == -1 
            || end_num < start_num
            || value_num > MAX_VAL || value_num == -1) { // error occurred ; invalid boundary || invalid hexadecimal number || start is bigger then end
                printf("Invalid Arguments!\n");
                continue;
            }

            push_history(cmd); // since cmd is valid, push it to the history
            cmd_fill(start_num, end_num, value_num);
        }

        else if (!strcmp(cmd_ptr, "reset")) { // when cmd is reset
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            push_history(cmd); // since cmd is valid, push it to the history
            cmd_reset();
        }

        else if (!strcmp(cmd_ptr, "opcode")) { // when cmd is opcode mnemonic
            char* mnemonic = strtok(NULL, DELIMITER);
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            if (cmd_opcode(mnemonic)) push_history(cmd); // since cmd is valid, push it to the history

        }

        else if (!strcmp(cmd_ptr, "opcodelist")) { // when cmd is opcodelist
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            if (cmd_opcodelist()) push_history(cmd); // since cmd is valid, push it to the history
        }

        else if (!strcmp(cmd_ptr, "type")) { // when cmd is type filename
            char* filename = strtok(NULL, DELIMITER);
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            if (cmd_type(filename)) push_history(cmd); // since cmd is valid, push it to the history
        }

        else if (!strcmp(cmd_ptr, "assemble")) { // when cmd is assemble filename
            char* filename = strtok(NULL, DELIMITER);
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            if ((cmd_assemble(filename))) {
                push_history(cmd); // since cmd is valid, push it to the history
                printf("\x1b[32mSuccessfully\x1b[0m %s\n", cmd);
            }
        }

        else if (!strcmp(cmd_ptr, "symbol")) { // when cmd is symbol
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue; 
            }
            cmd_symbol();
            push_history(cmd);
        }

        else if (!strcmp(cmd_ptr, "progaddr")) { // when cmd is progaddr
            char* address = strtok(NULL, DELIMITER);
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue;
            }
            cmd_progaddr(address);
            push_history(cmd);
        }

        else if (!strcmp(cmd_ptr, "loader")) { // when cmd is loader
            char* filename1 = strtok(NULL, DELIMITER);
            char* filename2 = strtok(NULL, DELIMITER);
            char* filename3 = strtok(NULL, DELIMITER);
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue;
            }
            if (cmd_loader(filename1, filename2, filename3)) {
                push_history(cmd); // since cmd is valid, push it to the history
            }
        }

        else if (!strcmp(cmd_ptr, "bp")) { // when cmd is bp
            char* option = strtok(NULL, DELIMITER);
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue;
            }
            cmd_bp(option);
            push_history(cmd);
        }

        else if (!strcmp(cmd_ptr, "run")) { // when cmd is run
            if ((eos = strtok(NULL, DELIMITER))) { // cmd is invalid
                printf("Too Many Arguments!\n");
                continue;
            }
            cmd_run();
            push_history(cmd); // assume that there's no error
        }

        else printf("Invalid Command!\n"); // Invalid cmd
    }
    return 0;
}
