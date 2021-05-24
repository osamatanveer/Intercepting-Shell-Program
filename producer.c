#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char alphanum[] ="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    time_t t;
    srand((unsigned) time(&t));

    for (int i = 0; i < atoi(argv[1]); i++) {
        printf("%c", alphanum[rand() % strlen(alphanum)]);
    }   

    return 0;
}