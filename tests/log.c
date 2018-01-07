#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

/* Test(log, stream) */
/*
 * This test works for both log_stream() and log_open().
 */
Test(log, open)
{
    log_open(stdin);
    cr_assert_eq(log_stream(), stdin);
    log_open(stdout);
    cr_assert_eq(log_stream(), stdout);
}

Test(log, close)
{
    FILE *fp;

    fp = fopen("/tmp/test.log.txt", "w");
    log_open(fp);

    cr_assert_eq(log_close(), 0);
}

Test(log, write) {
    char message[] = "we write the logs";
    char message_buffer[128];
    FILE *fp;
    int message_length;

    message_length = strlen(message);

    log_open(NULL);
    log_write(0, message);
    log_close();

    fp = fopen(LOG_FILENAME, "r");
    cr_assert_neq(fp, NULL, "Unable to open " LOG_FILENAME);

    fread(message_buffer, sizeof(char), sizeof(message), fp);
    message_buffer[message_length] = '\0';
    cr_assert_str_eq(message_buffer, 
                     message, 
                     "log_write() did not write correct data ([%s], [%s])",
                     message_buffer,
                     message);

    fclose(fp);
    unlink(LOG_FILENAME);
}
