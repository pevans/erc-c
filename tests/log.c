#include <criterion/criterion.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "log.h"

Test(log, write) {
    char message[] = "we write the logs";
    char message_buffer[128];
    FILE *fp;
    int message_length;

    message_length = strlen(message);

    log_open();
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
