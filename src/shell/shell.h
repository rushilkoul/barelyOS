#ifndef SHELL_H
#define SHELL_H

#define BARELY_VERSION  "0.1"
#define MAX_ARGS        64
#define SHELL_MAX_INPUT  1024

typedef int (*builtin_fn)(int argc, char **argv);

typedef struct {
    const char *name;
    builtin_fn  fn;
    const char *help;
} builtin_t;

#endif /* SHELL_H */
