#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
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
-   #include <sys/stat.h>  — chmod(), stat()
-   #include <fcntl.h>     — open(), O_RDONLY, O_WRONLY, O_CREAT
-   #include <unistd.h>    — rename(), link(), symlink(), unlink()
 */

static int copy_file(const char *src, const char *dst)
{
    int in_fd = open(src, O_RDONLY);
    if (in_fd < 0)
        return -1;

    int out_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        close(in_fd);
        return -1;
    }

    char buf[4096];
    ssize_t n;
    while ((n = read(in_fd, buf, sizeof(buf))) > 0) {
        if (write(out_fd, buf, (size_t)n) != n) {
            close(in_fd);
            close(out_fd);
            return -1;
        }
    }

    close(in_fd);
    close(out_fd);
    return (n < 0) ? -1 : 0;
}

static int cmd_mv(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "mv: missing operand\n");
        return 1;
    }
    if (rename(argv[1], argv[2]) < 0) {
        if (errno == EXDEV) {
            if (copy_file(argv[1], argv[2]) < 0 || unlink(argv[1]) < 0) {
                perror("mv");
                return 1;
            }
            return 0;
        }
        perror("mv");
        return 1;
    }
    return 0;
}

static int cmd_cp(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "cp: missing operand\n");
        return 1;
    }
    if (copy_file(argv[1], argv[2]) < 0) {
        perror("cp");
        return 1;
    }
    return 0;
}

static int cmd_ln(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "ln: missing operand\n");
        return 1;
    }
    if (strcmp(argv[1], "-s") == 0) {
        if (argc < 4) {
            fprintf(stderr, "ln: missing operand\n");
            return 1;
        }
        if (symlink(argv[2], argv[3]) < 0) {
            perror("ln");
            return 1;
        }
        return 0;
    }
    if (link(argv[1], argv[2]) < 0) {
        perror("ln");
        return 1;
    }
    return 0;
}

static int cmd_chmod(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "chmod: missing operand\n");
        return 1;
    }
    mode_t mode = (mode_t)strtol(argv[1], NULL, 8);
    if (chmod(argv[2], mode) < 0) {
        perror("chmod");
        return 1;
    }
    return 0;
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
    if (argc < 2) {
        fprintf(stderr, "cat: missing operand\n");
        return 1;
    }

    int result = 0;
    char buf[4096];
    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "rb");
        if (!file) {
            perror("cat");
            result = 1;
            continue;
        }

        size_t bytes_read;
        while ((bytes_read = fread(buf, 1, sizeof(buf), file)) > 0) {
            if (fwrite(buf, 1, bytes_read, stdout) != bytes_read) {
                perror("cat");
                result = 1;
                break;
            }
        }
        if (ferror(file)) {
            perror("cat");
            result = 1;
        }
        if (fclose(file) != 0) {
            perror("cat");
            result = 1;
        }
    }
    return result;
}

static int parse_line_count(const char *text, const char *command,
                            unsigned long *line_count)
{
    char *end;
    errno = 0;
    unsigned long count = strtoul(text, &end, 10);
    if (text[0] == '-' || text[0] == '\0' || *end != '\0' ||
        end == text || errno == ERANGE) {
        fprintf(stderr, "%s: invalid number of lines\n", command);
        return 1;
    }
    *line_count = count;
    return 0;
}

static int cmd_head(int argc, char **argv)
{
    unsigned long line_count = 10;
    const char *path;
    if (argc >= 2 && strcmp(argv[1], "-n") == 0) {
        if (argc < 4) {
            fprintf(stderr, "head: missing operand\n");
            return 1;
        }
        if (parse_line_count(argv[2], "head", &line_count) != 0)
            return 1;
        path = argv[3];
    } else {
        if (argc < 2) {
            fprintf(stderr, "head: missing operand\n");
            return 1;
        }
        path = argv[1];
    }

    FILE *file = fopen(path, "rb");
    if (!file) {
        perror("head");
        return 1;
    }

    char *line = NULL;
    size_t capacity = 0;
    unsigned long lines_printed = 0;
    ssize_t length;
    int result = 0;
    while (lines_printed < line_count &&
           (length = getline(&line, &capacity, file)) != -1) {
        if (fwrite(line, 1, (size_t)length, stdout) != (size_t)length) {
            perror("head");
            result = 1;
            break;
        }
        lines_printed++;
    }
    if (ferror(file)) {
        perror("head");
        result = 1;
    }
    free(line);
    if (fclose(file) != 0) {
        perror("head");
        result = 1;
    }
    return result;
}

static int cmd_tail(int argc, char **argv)
{
    unsigned long line_count = 10;
    const char *path;
    if (argc >= 2 && strcmp(argv[1], "-n") == 0) {
        if (argc < 4) {
            fprintf(stderr, "tail: missing operand\n");
            return 1;
        }
        if (parse_line_count(argv[2], "tail", &line_count) != 0)
            return 1;
        path = argv[3];
    } else {
        if (argc < 2) {
            fprintf(stderr, "tail: missing operand\n");
            return 1;
        }
        path = argv[1];
    }

    FILE *file = fopen(path, "rb");
    if (!file) {
        perror("tail");
        return 1;
    }

    struct tail_line {
        char *data;
        size_t length;
    };
    size_t capacity = (size_t)line_count;
    if ((unsigned long)capacity != line_count ||
        capacity > (size_t)-1 / sizeof(struct tail_line)) {
        fprintf(stderr, "tail: invalid number of lines\n");
        fclose(file);
        return 1;
    }
    struct tail_line *lines = capacity
        ? calloc(capacity, sizeof(*lines)) : NULL;
    if (capacity && !lines) {
        perror("tail");
        fclose(file);
        return 1;
    }

    char *line = NULL;
    size_t line_capacity = 0;
    size_t stored = 0;
    size_t next = 0;
    ssize_t length;
    int result = 0;
    while ((length = getline(&line, &line_capacity, file)) != -1) {
        if (capacity == 0)
            continue;

        char *copy = malloc((size_t)length);
        if (!copy) {
            perror("tail");
            result = 1;
            break;
        }
        memcpy(copy, line, (size_t)length);
        free(lines[next].data);
        lines[next].data = copy;
        lines[next].length = (size_t)length;
        next = (next + 1) % capacity;
        if (stored < capacity)
            stored++;
    }
    if (ferror(file)) {
        perror("tail");
        result = 1;
    }
    free(line);
    if (fclose(file) != 0) {
        perror("tail");
        result = 1;
    }

    size_t start = (stored == capacity && capacity != 0) ? next : 0;
    for (size_t i = 0; i < stored; i++) {
        size_t index = (start + i) % capacity;
        if (fwrite(lines[index].data, 1, lines[index].length, stdout) !=
            lines[index].length) {
            perror("tail");
            result = 1;
            break;
        }
    }
    for (size_t i = 0; i < stored; i++)
        free(lines[i].data);
    free(lines);
    return result;
}

static int cmd_wc(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "wc: missing operand\n");
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("wc");
        return 1;
    }

    unsigned long newlines = 0;
    unsigned long words = 0;
    unsigned long bytes = 0;
    int in_word = 0;
    int ch;
    while ((ch = fgetc(file)) != EOF) {
        bytes++;
        if (ch == '\n')
            newlines++;
        if (isspace((unsigned char)ch)) {
            in_word = 0;
        } else if (!in_word) {
            words++;
            in_word = 1;
        }
    }

    int result = 0;
    if (ferror(file)) {
        perror("wc");
        result = 1;
    } else {
        printf("%lu %lu %lu %s\n", newlines, words, bytes, argv[1]);
    }
    if (fclose(file) != 0) {
        perror("wc");
        result = 1;
    }
    return result;
}

static int cmd_echo(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {
        if (i > 1)
            putchar(' ');
        fputs(argv[i], stdout);
    }
    putchar('\n');
    if (ferror(stdout)) {
        perror("echo");
        return 1;
    }
    return 0;
}

static int cmd_pwd(int argc, char **argv)
{
    (void)argc; (void)argv;
    char buf[PATH_MAX];
    if (!getcwd(buf, sizeof(buf))) {
        perror("pwd");
        return 1;
    }
    if (puts(buf) == EOF) {
        perror("pwd");
        return 1;
    }
    return 0;
}
