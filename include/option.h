#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <stdio.h>

/*
 * These are the most possible number of disk inputs we can accept
 */
#define OPTION_MAX_DISKS 2

extern const char *option_get_error();
extern FILE *option_get_input(int);
extern int option_parse(int, char **);
extern void option_print_help();
extern int option_read_file(int, const char *);
extern void option_set_error(const char *);
extern void option_set_input(int, FILE *);

#endif
