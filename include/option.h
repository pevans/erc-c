#ifndef _OPTIONS_H_
#define _OPTIONS_H_

#include <stdio.h>

/*
 * These are the most possible number of disk inputs we can accept
 */
#define OPTION_MAX_DISKS 2

extern FILE *option_get_input(int);
extern const char *option_get_error();
extern int option_get_height();
extern int option_get_width();
extern int option_parse(int, char **);
extern int option_read_file(int, const char *);
extern int option_set_size(const char *);
extern void option_print_help();
extern void option_set_error(const char *);
extern void option_set_input(int, FILE *);

#endif
