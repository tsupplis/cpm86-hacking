#include <stdio.h>

int main (int argc, char *argv[], char **envp) {
    char **e=envp;
    /*
    int i;

    for(i=1;i<argc;i++) {
        putenv(argv[i]);
    }*/
    while(*e) {
        printf("%s\n",*e);
        e++;
    }
    return 0;
}
