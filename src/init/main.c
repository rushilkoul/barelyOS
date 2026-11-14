#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("barelyOS: custom init running!\n");
    printf("PID: %d\n", getpid());

    while (1) {
        sleep(10);
    }

    return 0;
}
