#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/mount.h>

int main(void) {
    printf("barelyOS: init started. PID = %d\n", getpid());

    if (mount("proc", "/proc", "proc", 0, NULL) < 0) perror("mount /proc");
    if (mount("sysfs", "/sys", "sysfs", 0, NULL) < 0) perror("mount /sys");
    if (mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) < 0 && errno != EBUSY)
        perror("mount /dev");
    

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
        
            execl("/bin/barelysh", "barelysh", (char *)NULL);
        
            perror("exec");
            _exit(127);
        }

        
        int status;
        pid_t child;

        while (1) {
            child = waitpid(-1, &status, 0); /* block until any child exits */

            if (child < 0) {
                if (errno == EINTR)
                    continue;
                perror("waitpid");
                break;
            }

            if (child == pid) {
                if (WIFEXITED(status))
                    printf("barelyOS: shell exited with status %d\n",
                           WEXITSTATUS(status));
                else if (WIFSIGNALED(status))
                    printf("barelyOS: shell killed by signal %d\n",
                           WTERMSIG(status));

                printf("barelyOS: restarting shell...\n");
                break;
            }

            /* reap any other stray children spawned by the shell */
            printf("barelyOS: reaped child PID %d\n", child);
        }

    }

    return 0;
}
