#include "utility.h"

int hexa_str_to_int(char* str, int idx) { // change hexadecimal string to integer
    int ret = 0;
    for (int i = idx, j = 1; i >= 0; i--, j *= 16) {
        if (str[i] >= '0' && str[i] <= '9')
            ret += (str[i] - '0') * j;
        else if (str[i] >= 'A' && str[i] <= 'F')
            ret += (str[i] - 'A' + 10) * j;
        else if (str[i] >= 'a' && str[i] <= 'f')
            ret += (str[i] - 'a' + 10) * j;
        else return -1; // error occurred ; invalid hexadecimal number
    }
    return ret;
}

int dec_str_to_int(char* str, int idx) { // change decimal string to integer
    int ret = 0;
    for (int i = idx, j = 1; i >= 0; i--, j *= 10) {
        if (str[i] >= '0' && str[i] <= '9')
            ret += (str[i] - '0') * j;
        else return -1;
    }
    return ret;
}
