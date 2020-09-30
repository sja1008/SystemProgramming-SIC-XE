#include "shell.h"

int cnt; // number of nodes in the history linked list
hptr head; // point head node of the history linked list

void cmd_help() { // when cmd is h[elp]
    printf("h[elp]\n");
    printf("d[ir]\n");
    printf("q[uit]\n");
    printf("hi[story]\n");
    printf("du[mp] [start, end]\n");
    printf("e[dit] address, value\n");
    printf("f[ill] start, end, value\n");
    printf("reset\n");
    printf("opcode mnemonic\n");
    printf("opcodelist\n");
    printf("assemble filename\n");
    printf("type filename\n");
    printf("symbol\n");
    printf("progaddr address\n");
    printf("loader object_filename1 [object_filename2 object_filename3]\n");
    printf("bp address\n");
    printf("bp clear\n");
    printf("bp\n");
    printf("run\n");
    return;
}

void cmd_dir() { // when cmd is d[ir]
    DIR *dir_ptr = opendir("."); // open the present working directory
    struct dirent *file = NULL; // to get a file name
    struct stat file_stat; // to get state of the file

    while ((file = readdir(dir_ptr))) { // read an entry of the directory
        lstat(file->d_name, &file_stat); // get state of the file from the file name
        if (S_ISDIR(file_stat.st_mode)) // when the file is directory
            printf("%s/\n", file->d_name);
        else if(file_stat.st_mode & S_IXUSR) /// when the file is executable file
            printf("%s*\n", file->d_name);
        else printf("%s\n", file->d_name); // otherwise
    }
    closedir(dir_ptr);
    return;
}

void push_history(char* cmd) { // push cmd to the history
    hptr tmp = (hptr)malloc(sizeof(HIST_NODE)); // create new node
    tmp->val_cnt = ++cnt; // increase the total number of nodes, and then store it as an order of the node
    strcpy(tmp->val_cmd, cmd); // store the cmd
    tmp->link = NULL;
    if (!head) head = tmp; // when the linked list is empty
    else {
        hptr w;
        for (w = head; w->link; w = w->link); // go to the last node
        w->link = tmp; // push current node to the end of the linked list
    }
    return;
}

void cmd_history(){ // when cmd is hi[story]
    for (hptr w = head; w; w = w->link) {
        printf("%-5d %s\n", w->val_cnt, w->val_cmd); // print contents of the histiory linked list in order
    }
    return;
}

int cmd_type(char* filename) { // when cmd is type filename
    struct stat file_stat; // to get state of the file
    char line[LINE_SIZE];

    FILE *fp = fopen(filename, "r"); // read file
    if (!fp) {
        printf("File Doesn't Exist!\n"); // error occurred ; no file
        return 0;
    }

    lstat(filename, &file_stat); // get state of the file from the file name
    if (S_ISDIR(file_stat.st_mode)) { // when the file is directory
        fclose(fp);
        printf("The File is Directory!\n");
        return 0;
    }
    while (fgets(line, LINE_SIZE, fp))
        printf("%s", line);
    return 1;
}
