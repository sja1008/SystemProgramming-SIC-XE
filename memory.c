#include "memory.h"

unsigned char mem[MEM_SIZE]; // memory

void cmd_dump(int start, int end) { // when cmd is du[mp] [start [end]]
    for (int i = start-(start%16); i <= end; i+=16) { // note that memory addr of left column is multiples of 16
        printf("%05X", i); // print left column that is memory address
        for (int j = 0; j < 16; j++) { // print middle column that is hexa value of memory addr
            printf(" ");
            if (i+j < start || i+j > end) {
                printf("  ");
                continue;
            }
            printf("%02X", mem[i+j]);
        }
        printf(" ; ");
        for (int j = 0; j < 16; j++) { // print right column that is character value of memory addr
            if (i+j < start || i+j > end || mem[i+j] < 0x20 || mem[i+j] > 0x7E) {
                printf(".");
                continue;
            }
            printf("%c", mem[i+j]);
        }
        printf("\n");
    }
    return;
}

void cmd_edit(int address, int value) { // when cmd is e[dit]
    mem[address] = value; // change the value
    return;
}

void cmd_fill(int start, int end, int value) { // when cmd if f[ill]
    for (int i = start; i <= end; i++) // change the value from start to end
        mem[i] = value;
    return;
}

void cmd_reset() { // when cmd is reset
    memset(mem, 0, sizeof(mem)); // change all value of the memory to zero
}
