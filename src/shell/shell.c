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
#include <dirent.h>
#include <fnmatch.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/reboot.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>

#include "shell.h"

static int builtin_cd(int argc, char **argv);
static int builtin_exit(int argc, char **argv);
static int builtin_help(int argc, char **argv);
static int builtin_bash(int argc, char **argv);
static int builtin_poweroff(int argc, char **argv);
static int builtin_reboot(int argc, char **argv);
static int builtin_barely_info(int argc, char **argv);
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

static void mode_string(mode_t mode, char out[11]);
static const char *path_basename(const char *path);
static int list_directory(const char *path, int show_all, int long_format);
static int find_matches(const char *path, const char *pattern);
static int print_tree(const char *path, const char *prefix);


static const builtin_t builtins[] = {
    { "cd",       builtin_cd,       "change directory"                        },
    { "exit",     builtin_exit,     "exit the shell"                          },
    { "help",     builtin_help,     "list built-in commands"                  },
    { "bash",     builtin_bash,     "run a command via /bin/sh, bypassing us" },
    { "poweroff", builtin_poweroff, "flush disks and shut down machine"       },
    { "reboot",   builtin_reboot,   "flush disks and restart machine"         },
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

static int builtin_bash(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "bash: usage: bash <command> [args...]\n");
        return 1;
    }

    size_t total = 0;
    for (int i = 1; i < argc; i++)
        total += strlen(argv[i]) + 1;

    char *cmd = malloc(total);
    if (!cmd) { perror("bash"); return 1; }

    cmd[0] = '\0';
    for (int i = 1; i < argc; i++) {
        if (i > 1) strcat(cmd, " ");
        strcat(cmd, argv[i]);
    }

    pid_t pid = fork();
    if (pid < 0) { perror("bash"); free(cmd); return 1; }
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", cmd, (char *)NULL);
        perror("bash");
        _exit(127);
    }
    int status;
    waitpid(pid, &status, 0);
    free(cmd);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

static int builtin_poweroff(int argc, char **argv)
{
    (void)argc; (void)argv;
    printf("barelyOS: syncing disks and powering off...\n");
    sync();
    if (reboot(RB_POWER_OFF) < 0) {
        perror("poweroff");
        return 1;
    }
    return 0;
}

static int builtin_reboot(int argc, char **argv)
{
    (void)argc; (void)argv;
    printf("barelyOS: syncing disks and rebooting...\n");
    sync();
    if (reboot(RB_AUTOBOOT) < 0) {
        perror("reboot");
        return 1;
    }
    return 0;
}

int main(void)
{
    char  line[SHELL_MAX_INPUT];
    char *argv[MAX_ARGS];
    char  cwd[PATH_MAX];
    const char *home = getenv("HOME");
    if (!home) home = "/root";
    setenv("HOME", home, 0);

    if (getcwd(cwd, sizeof(cwd)) && strcmp(cwd, "/") == 0) {
        chdir(home);
    }

    printf("barelyOS shell v" BARELY_VERSION " | type 'help' for built-ins\n\n");

    while (1) {
        const char *display;
        char rel[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) {
            display = "?";
        } else if (strncmp(cwd, home, strlen(home)) == 0 &&
                   (cwd[strlen(home)] == '/' || cwd[strlen(home)] == '\0')) {
            snprintf(rel, sizeof(rel), "~%s", cwd + strlen(home));
            display = rel;
        } else {
            display = cwd;
        }

        printf("barelysh:%s$ ", display);
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

static int cmd_ls(int argc, char **argv)
{
    int show_all = 0, long_format = 0;
    const char *path = ".";

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            for (const char *opt = argv[i] + 1; *opt; opt++) {
                if (*opt == 'a') show_all = 1;
                else if (*opt == 'l') long_format = 1;
                else {
                    fprintf(stderr, "ls: invalid option -- '%c'\n", *opt);
                    return 1;
                }
            }
        } else {
            path = argv[i];
        }
    }

    struct stat st;
    if (stat(path, &st) < 0) {
        perror("ls");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        if (long_format) {
            char mode[11];
            mode_string(st.st_mode, mode);
            printf("%s %3lu %s %s %8lld %.12s %s\n", mode,
                   (unsigned long)st.st_nlink,
                   getpwuid(st.st_uid) ? getpwuid(st.st_uid)->pw_name : "?",
                   getgrgid(st.st_gid) ? getgrgid(st.st_gid)->gr_name : "?",
                   (long long)st.st_size,
                   ctime(&st.st_mtime) ? ctime(&st.st_mtime) + 4 : "unknown",
                   path_basename(path));
        } else {
            puts(path_basename(path));
        }
        return 0;
    }
    return list_directory(path, show_all, long_format);
}

static int cmd_find(int argc, char **argv)
{
    const char *root = ".";
    const char *pattern = "*";

    if (argc > 1) root = argv[1];
    if (argc > 2) {
        if (strcmp(argv[2], "-name") == 0 && argc > 3)
            pattern = argv[3];
        else
            pattern = argv[2];
    }
    return find_matches(root, pattern) < 0 ? 1 : 0;
}

static int cmd_stat(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "stat: missing operand\n");
        return 1;
    }
    int failed = 0;
    for (int i = 1; i < argc; i++) {
        struct stat st;
        if (lstat(argv[i], &st) < 0) {
            perror(argv[i]);
            failed = 1;
            continue;
        }
        char mode[11];
        mode_string(st.st_mode, mode);
        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        char *modified = ctime(&st.st_mtime);
        printf("  File: %s\n", argv[i]);
        printf("  Size: %lld\tBlocks: %lld\tIO Block: %ld\n",
               (long long)st.st_size, (long long)st.st_blocks,
               (long)st.st_blksize);
        printf("Device: %u\tInode: %llu\tLinks: %lu\n",
               (unsigned int)st.st_dev, (unsigned long long)st.st_ino,
               (unsigned long)st.st_nlink);
        printf("Access: (%04o/%s)\tUid: (%u/%s)\tGid: (%u/%s)\n",
               (unsigned int)(st.st_mode & 07777), mode,
               (unsigned int)st.st_uid, pw ? pw->pw_name : "?",
               (unsigned int)st.st_gid, gr ? gr->gr_name : "?");
        printf("Modify: %s", modified ? modified : "unknown\n");
    }
    return failed;
}

static int cmd_tree(int argc, char **argv)
{
    const char *root = (argc > 1) ? argv[1] : ".";
    struct stat st;
    if (lstat(root, &st) < 0) {
        perror("tree");
        return 1;
    }
    puts(root);
    if (!S_ISDIR(st.st_mode)) return 0;
    return print_tree(root, "") < 0 ? 1 : 0;
}

static const char *path_basename(const char *path)
{
    const char *base = strrchr(path, '/');
    return base ? base + 1 : path;
}

static void mode_string(mode_t mode, char out[11])
{
    out[0] = S_ISDIR(mode) ? 'd' : S_ISLNK(mode) ? 'l' :
             S_ISCHR(mode) ? 'c' : S_ISBLK(mode) ? 'b' :
             S_ISFIFO(mode) ? 'p' : S_ISSOCK(mode) ? 's' : '-';
    const mode_t bits[] = { S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                            S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH };
    const char chars[] = "rwxrwxrwx";
    for (int i = 0; i < 9; i++) out[i + 1] = (mode & bits[i]) ? chars[i] : '-';
    if (mode & S_ISUID) out[3] = (mode & S_IXUSR) ? 's' : 'S';
    if (mode & S_ISGID) out[6] = (mode & S_IXGRP) ? 's' : 'S';
    if (mode & S_ISVTX) out[9] = (mode & S_IXOTH) ? 't' : 'T';
    out[10] = '\0';
}

static int list_directory(const char *path, int show_all, int long_format)
{
    DIR *dir = opendir(path);
    if (!dir) { perror(path); return 1; }
    int failed = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!show_all && entry->d_name[0] == '.') continue;
        if (!long_format) {
            printf("%s\n", entry->d_name);
            continue;
        }
        char full[4096];
        int n = snprintf(full, sizeof(full), "%s/%s", path, entry->d_name);
        struct stat st;
        if (n < 0 || (size_t)n >= sizeof(full) || lstat(full, &st) < 0) {
            perror(entry->d_name);
            failed = 1;
            continue;
        }
        char mode[11];
        mode_string(st.st_mode, mode);
        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        char *date = ctime(&st.st_mtime);
        printf("%s %3lu %-8s %-8s %8lld %.12s %s", mode,
               (unsigned long)st.st_nlink, pw ? pw->pw_name : "?",
               gr ? gr->gr_name : "?", (long long)st.st_size,
               date ? date + 4 : "unknown", entry->d_name);
        if (S_ISLNK(st.st_mode)) {
            char target[4096];
            ssize_t len = readlink(full, target, sizeof(target) - 1);
            if (len >= 0) { target[len] = '\0'; printf(" -> %s", target); }
        }
        putchar('\n');
    }
    if (closedir(dir) < 0) { perror(path); failed = 1; }
    return failed;
}

static int find_matches(const char *path, const char *pattern)
{
    struct stat st;
    if (lstat(path, &st) < 0) { perror(path); return -1; }
    if (fnmatch(pattern, path_basename(path), 0) == 0) puts(path);
    if (!S_ISDIR(st.st_mode)) return 0;

    DIR *dir = opendir(path);
    if (!dir) { perror(path); return -1; }
    int failed = 0;
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        char child[4096];
        int n = snprintf(child, sizeof(child), "%s%s%s", path,
                         strcmp(path, "/") == 0 ? "" : "/", entry->d_name);
        if (n < 0 || (size_t)n >= sizeof(child)) {
            fprintf(stderr, "find: path too long: %s/%s\n", path, entry->d_name);
            failed = 1;
        } else if (find_matches(child, pattern) < 0) {
            failed = 1;
        }
    }
    if (closedir(dir) < 0) { perror(path); failed = 1; }
    return failed ? -1 : 0;
}

static int print_tree(const char *path, const char *prefix)
{
    DIR *dir = opendir(path);
    if (!dir) { perror(path); return -1; }
    struct dirent *entry;
    char **names = NULL;
    size_t count = 0, capacity = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        if (count == capacity) {
            size_t next = capacity ? capacity * 2 : 16;
            char **grown = realloc(names, next * sizeof(*names));
            if (!grown) { perror("tree"); closedir(dir); goto fail; }
            names = grown;
            capacity = next;
        }
        names[count] = strdup(entry->d_name);
        if (!names[count]) { perror("tree"); closedir(dir); goto fail; }
        count++;
    }
    if (closedir(dir) < 0) { perror(path); goto fail; }
    for (size_t i = 0; i < count; i++) {
        for (size_t j = i + 1; j < count; j++) {
            if (strcmp(names[i], names[j]) > 0) {
                char *tmp = names[i]; names[i] = names[j]; names[j] = tmp;
            }
        }
    }
    for (size_t i = 0; i < count; i++) {
        int last = i + 1 == count;
        char child[4096], next_prefix[4096];
        int n = snprintf(child, sizeof(child), "%s%s%s", path,
                         strcmp(path, "/") == 0 ? "" : "/", names[i]);
        if (n < 0 || (size_t)n >= sizeof(child)) {
            fprintf(stderr, "tree: path too long: %s/%s\n", path, names[i]);
            goto fail;
        }
        printf("%s%s%s\n", prefix, last ? "└── " : "├── ", names[i]);
        struct stat st;
        if (lstat(child, &st) == 0 && S_ISDIR(st.st_mode)) {
            n = snprintf(next_prefix, sizeof(next_prefix), "%s%s",
                         prefix, last ? "    " : "│   ");
            if (n < 0 || (size_t)n >= sizeof(next_prefix) ||
                print_tree(child, next_prefix) < 0)
                goto fail;
        }
        free(names[i]);
        names[i] = NULL;
    }
    free(names);
    return 0;
fail:
    for (size_t i = 0; i < count; i++) free(names[i]);
    free(names);
    return -1;
}

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
    char *end;
    mode_t mode = (mode_t)strtol(argv[1], &end, 8);
    if (*end != '\0' || end == argv[1]) {
        fprintf(stderr, "chmod: invalid mode: %s\n", argv[1]);
        return 1;
    }
    if (chmod(argv[2], mode) < 0) {
        perror("chmod");
        return 1;
    }
    return 0;
}

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
