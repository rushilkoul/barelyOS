#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>

#include "shell.h"

static int builtin_cd(int argc, char **argv);
static int builtin_exit(int argc, char **argv);
static int builtin_help(int argc, char **argv);
static int cmd_ls(int argc, char **argv);
static int cmd_find(int argc, char **argv);
static int cmd_stat(int argc, char **argv);
static int cmd_tree(int argc, char **argv);
static int cmd_mv(int argc, char **argv);
static int cmd_cp(int argc, char **argv);
static int cmd_ln(int argc, char **argv);
static int cmd_chmod(int argc, char **argv);
static int cmd_mkdir(int argc, char **argv);
static int cmd_rmdir(int argc, char **argv);
static int cmd_rm(int argc, char **argv);
static int cmd_touch(int argc, char **argv);
static int cmd_cat(int argc, char **argv);
static int cmd_head(int argc, char **argv);
static int cmd_tail(int argc, char **argv);
static int cmd_wc(int argc, char **argv);
static int cmd_echo(int argc, char **argv);
static int cmd_pwd(int argc, char **argv);


static const builtin_t builtins[] = {
    { "cd",     builtin_cd,    "change directory"            },
    { "exit",   builtin_exit,  "exit the shell"              },
    { "help",   builtin_help,  "list built-in commands"      },
    { "ls",     cmd_ls,        "list directory contents"     },
    { "find",   cmd_find,      "search for files"            },
    { "stat",   cmd_stat,      "display file status"         },
    { "tree",   cmd_tree,      "display directory tree"      },
    { "mv",     cmd_mv,        "move or rename a file"       },
    { "cp",     cmd_cp,        "copy a file"                 },
    { "ln",     cmd_ln,        "create a link"               },
    { "chmod",  cmd_chmod,     "change file permissions"     },
    { "mkdir",  cmd_mkdir,     "create a directory"          },
    { "rmdir",  cmd_rmdir,     "remove an empty directory"   },
    { "rm",     cmd_rm,        "remove a file or directory"  },
    { "touch",  cmd_touch,     "create or update a file"     },
    { "cat",    cmd_cat,       "print file contents"         },
    { "head",   cmd_head,      "print first N lines"         },
    { "tail",   cmd_tail,      "print last N lines"          },
    { "wc",     cmd_wc,        "count lines, words, bytes"   },
    { "echo",   cmd_echo,      "print arguments"             },
    { "pwd",    cmd_pwd,       "print working directory"     },
};

static const int n_builtins = (int)(sizeof(builtins) / sizeof(builtins[0]));

static int tokenize(char *line, char **argv, int max_args)
{
    int argc = 0;
    char *tok = strtok(line, " \t\n");
    while (tok && argc < max_args - 1) {
        argv[argc++] = tok;
        tok = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    return argc;
}

static void exec_external(char **argv)
{
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return; }
    if (pid == 0) {
        execvp(argv[0], argv);
        if (errno == ENOENT)
            fprintf(stderr, "barely: %s: command not found\n", argv[0]);
        else
            perror(argv[0]);
        _exit(127);
    }
    int status;
    waitpid(pid, &status, 0);
}

static int builtin_cd(int argc, char **argv)
{
    const char *dir = (argc > 1) ? argv[1] : getenv("HOME");
    if (!dir) dir = "/";
    if (chdir(dir) < 0) perror("cd");
    return 0;
}

static int builtin_exit(int argc, char **argv)
{
    exit((argc > 1) ? atoi(argv[1]) : 0);
}

static int builtin_help(int argc, char **argv)
{
    (void)argc; (void)argv;
    printf("barely shell v" BARELY_VERSION " — built-ins:\n\n");
    for (int i = 0; i < n_builtins; i++)
        printf("  %-8s  %s\n", builtins[i].name, builtins[i].help);
    printf("\nany other command is passed to the system via PATH.\n");
    return 0;
}

int main(void)
{
    char  line[MAX_INPUT];
    char *argv[MAX_ARGS];

    printf("barelyOS shell v" BARELY_VERSION " | type 'help' for built-ins\n\n");

    while (1) {
        printf("barelysh$ ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) { printf("\n"); break; }

        int argc = tokenize(line, argv, MAX_ARGS);
        if (argc == 0) continue;

        int found = 0;
        for (int i = 0; i < n_builtins; i++) {
            if (strcmp(argv[0], builtins[i].name) == 0) {
                builtins[i].fn(argc, argv);
                found = 1;
                break;
            }
        }
        if (!found) exec_external(argv);
    }
    return 0;
}

/* ── Shreyansh: ls, find, stat, tree ───────────────────────────────────── */
/* Useful headers:
 *   #include <dirent.h>    — opendir, readdir, closedir
 *   #include <sys/stat.h>  — stat(), struct stat
 *   #include <pwd.h>       — getpwuid()
 *   #include <grp.h>       — getgrgid()
 */

static int cmd_ls(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: list contents of argv[1] (default: ".")
     * Stretch: -l long format, -a show hidden files */
    fprintf(stderr, "ls: not yet implemented\n");
    return 1;
}

static int cmd_find(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: recursively search argv[1] for name matching argv[2]
     * Hint: recursive opendir/readdir, fnmatch() for pattern matching */
    fprintf(stderr, "find: not yet implemented\n");
    return 1;
}

static int cmd_stat(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: print size, permissions, timestamps of argv[1]
     * Hint: stat(), struct stat */
    fprintf(stderr, "stat: not yet implemented\n");
    return 1;
}

static int cmd_tree(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: print directory tree rooted at argv[1] (default: ".")
     * Hint: recursive opendir/readdir, indent by depth */
    fprintf(stderr, "tree: not yet implemented\n");
    return 1;
}

/* ── Kaustubh: mv, cp, ln, chmod ───────────────────────────────────────── */
/* Useful headers:
 *   #include <sys/stat.h>  — chmod(), stat()
 *   #include <fcntl.h>     — open(), O_RDONLY, O_WRONLY, O_CREAT
 *   #include <unistd.h>    — rename(), link(), symlink(), unlink()
 */

static int cmd_mv(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: rename(argv[1], argv[2])
     * Cross-filesystem: copy + unlink */
    fprintf(stderr, "mv: not yet implemented\n");
    return 1;
}

static int cmd_cp(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: open both files, read/write in a loop
     * Stretch: -r for recursive directory copy */
    fprintf(stderr, "cp: not yet implemented\n");
    return 1;
}

static int cmd_ln(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: link() for hard, symlink() for soft (-s flag) */
    fprintf(stderr, "ln: not yet implemented\n");
    return 1;
}

static int cmd_chmod(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: strtol(argv[1], NULL, 8) to parse octal mode, then chmod() */
    fprintf(stderr, "chmod: not yet implemented\n");
    return 1;
}

/* ── Kisna: mkdir, rmdir, rm, touch ────────────────────────────────────── */
/* Useful headers:
 *   #include <sys/stat.h>  — mkdir()
 *   #include <unistd.h>    — rmdir(), unlink()
 *   #include <fcntl.h>     — open(), O_CREAT
 *   #include <utime.h>     — utime()
 */

static int cmd_mkdir(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "mkdir: missing operand\n");
        return 1;
    }
    if (mkdir(argv[1], 0755) < 0) {
        perror("mkdir");
        return 1;
    }
    return 0;
}

static int cmd_rmdir(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "rmdir: missing operand\n");
        return 1;
    }
    if (rmdir(argv[1]) < 0) {
        perror("rmdir");
        return 1;
    }
    return 0;
}

static int cmd_rm(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "rm: missing operand\n");
        return 1;
    }
    if (unlink(argv[1]) < 0) {
        perror("rm");
        return 1;
    }
    return 0;
}

static int cmd_touch(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "touch: missing operand\n");
        return 1;
    }

    int fd = open(argv[1], O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        perror("touch");
        return 1;
    }
    if (close(fd) < 0) {
        perror("touch");
        return 1;
    }
    if (utime(argv[1], NULL) < 0) {
        perror("touch");
        return 1;
    }
    return 0;
}

/* ── Abhigya: cat, head, tail, wc, echo, pwd ───────────────────────────── */
/* Useful headers:
 *   #include <stdio.h>     — fopen, fgets, fclose
 *   #include <unistd.h>    — getcwd()
 *   #include <string.h>    — strlen()
 *   #include <limits.h>    — PATH_MAX
 */

static int cmd_cat(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: fopen(argv[1], "r"), fgets loop, fclose
     * Stretch: multiple files (loop argv[1..argc-1]) */
    fprintf(stderr, "cat: not yet implemented\n");
    return 1;
}

static int cmd_head(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: print first N lines of argv[1], default N=10
     * Support -n <N> to override */
    fprintf(stderr, "head: not yet implemented\n");
    return 1;
}

static int cmd_tail(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: print last N lines, default N=10
     * Hint: circular buffer of N lines, print after EOF */
    fprintf(stderr, "tail: not yet implemented\n");
    return 1;
}

static int cmd_wc(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: count and print lines, words, bytes in argv[1]
     * Hint: fgetc loop — track newlines, whitespace transitions, bytes */
    fprintf(stderr, "wc: not yet implemented\n");
    return 1;
}

static int cmd_echo(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: print argv[1..argc-1] separated by spaces + newline */
    fprintf(stderr, "echo: not yet implemented\n");
    return 1;
}

static int cmd_pwd(int argc, char **argv)
{
    (void)argc; (void)argv;
    /* TODO: char buf[PATH_MAX]; getcwd(buf, sizeof(buf)); puts(buf); */
    fprintf(stderr, "pwd: not yet implemented\n");
    return 1;
}
