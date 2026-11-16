#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <signal.h>

static volatile sig_atomic_t g_shutdown_cmd = 0;

static void handle_shutdown_sig(int sig) {
    if (sig == SIGUSR1 || sig == SIGPWR) {
        g_shutdown_cmd = RB_POWER_OFF;
    } else if (sig == SIGUSR2 || sig == SIGINT || sig == SIGTERM) {
        g_shutdown_cmd = RB_AUTOBOOT;
    }
}

int main(void) {
    signal(SIGUSR1, handle_shutdown_sig);
    signal(SIGUSR2, handle_shutdown_sig);
    signal(SIGPWR,  handle_shutdown_sig);
    signal(SIGTERM, handle_shutdown_sig);

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

            struct termios tio;
            if (tcgetattr(tty, &tio) == 0) {
                tio.c_lflag |= (ICANON | ECHO | ECHOE | ECHOK | ECHONL);
                tio.c_cc[VERASE] = 0x08;
                tcsetattr(tty, TCSANOW, &tio);
            }
        
            dup2(tty, STDIN_FILENO);
            dup2(tty, STDOUT_FILENO);
            dup2(tty, STDERR_FILENO);
        
            if (tty > STDERR_FILENO)
                close(tty);

            setenv("TERM", "xterm-256color", 0);
            setenv("PATH", "/bin:/sbin:/usr/bin:/usr/sbin", 0);
        
            execl("/bin/barelysh", "barelysh", (char *)NULL);
        
            perror("exec");
            _exit(127);
        }

        
        int status;
        pid_t child;

        if (g_shutdown_cmd) {
            printf("barelyOS: shutdown requested, syncing disks...\n");
            sync();
            reboot(g_shutdown_cmd);
            while (1) pause();
        }

        while (1) {
            child = waitpid(-1, &status, 0);

            if (child < 0) {
                if (errno == EINTR) {
                    if (g_shutdown_cmd) break;
                    continue;
                }
                perror("waitpid");
                break;
            }

            if (g_shutdown_cmd) break;

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

            printf("barelyOS: reaped child PID %d\n", child);
        }

    }

    return 0;
}
