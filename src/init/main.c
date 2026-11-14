#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>


int main(void) {
    printf("barelyOS: init started. PID = %d\n", getpid());

    while (1) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            sleep(1);
            continue;
        }

        if (pid == 0) {
            if (setsid() < 0) {
                perror("setsid");
                _exit(1);
            }
        
            int tty = open("/dev/ttyS0", O_RDWR);
        
            if (tty < 0) {
                perror("open ttyS0");
                _exit(1);
            }
        
            if (ioctl(tty, TIOCSCTTY, 0) < 0) {
                perror("TIOCSCTTY");
                _exit(1);
            }
        
            dup2(tty, STDIN_FILENO);
            dup2(tty, STDOUT_FILENO);
            dup2(tty, STDERR_FILENO);
        
            if (tty > STDERR_FILENO)
                close(tty);
        
            execl("/bin/sh", "sh", (char *)NULL);
        
            perror("exec");
            _exit(127);
        }

        printf("barelyOS: spawned shell, PID = %d\n", pid);

        int status;
        pid_t result;

        do {
            result = waitpid(pid, &status, 0);
        } while (result < 0 && errno == EINTR);

        if (result < 0) {
            perror("waitpid");
        } else {
            printf("barelyOS: shell exited. Restarting...\n");
        }

        sleep(1);
    }

    return 0;
}
