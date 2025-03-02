#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void sigint_handler(int signum) {
    printf("SIGINT received");
    fflush(stdout);
}

int main() {
    signal(SIGINT, sigint_handler);
    while (1)
        getchar();
    return 0;
}
