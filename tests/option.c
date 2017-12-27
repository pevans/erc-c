#include <criterion/criterion.h>
#include <unistd.h>

#include "option.h"

static void
setup()
{
    option_set_error("");
    option_set_input(1, NULL);
    option_set_input(2, NULL);
}

static void
teardown()
{
    FILE *stream;

    for (int i = 1; i <= OPTION_MAX_DISKS; i++) {
        stream = option_get_input(i);

        if (stream 
            && stream != stdout 
            && stream != stderr 
            && stream != stdin
           ) {
            fclose(stream);
        }
    }
}

TestSuite(options, .init = setup, .fini = teardown);

Test(options, error)
{
    char *str = "hahaha FUN";

    cr_assert_str_empty(option_get_error());
    
    option_set_error(str);
    cr_assert_str_eq(option_get_error(), str);
}

Test(options, input)
{
    cr_assert_eq(option_get_input(1), NULL);
    cr_assert_eq(option_get_input(2), NULL);

    option_set_input(2, stdout);
    cr_assert_eq(option_get_input(2), stdout);

    option_set_input(3, stderr);
    cr_assert_eq(option_get_input(3), NULL);
}

Test(options, read_file)
{
    char *str = "so much FUN";
    char *bad_file = "/tmp/BLEH";
    char *file = "/tmp/erc-test.txt";
    char buf[256];

    cr_assert_eq(option_get_input(1), NULL);

    // Maybe we should use sterror(ENOENT)?
    cr_assert_eq(option_read_file(1, bad_file), 0);
    cr_assert_str_eq(option_get_error(), "--disk1: No such file or directory");

    option_set_error("");

    FILE *stream;
    stream = fopen(file, "w");
    cr_assert_neq(stream, NULL);
    fwrite(str, sizeof(char), strlen(str), stream);
    fclose(stream);

    option_read_file(1, file); 
    fread(buf, sizeof(char), 255, option_get_input(1));
    cr_assert_str_eq(buf, str);

    unlink(file);
}

/*
 * This test is really imperfect... that's because option_parse() does
 * a ton of stuff, and is quite complex. I'm punting a lot on the
 * complexity here, while pushing as much of the logic as I can into
 * other functions that are more easily testable.
 */
Test(options, parse)
{
    int argc = 2;
    char *argv[] = {
        "prog_name",
        "--disk1=etc",
    };

    cr_assert_eq(option_parse(argc, argv), 0);
}

/*
 * The get_width and get_height tests also implicitly test the
 * option_set_size() function (which is called by option_parse()).
 */
Test(options, get_width)
{
    int argc = 2;
    char *argv[] = {
        "prog_name",
        "--size=875x600",
    };

    cr_assert_eq(option_parse(argc, argv), 1);
    cr_assert_eq(option_get_width(), 875);
}

Test(options, get_height)
{
    int argc = 2;
    char *argv[] = {
        "prog_name",
        "--size=875x600",
    };

    cr_assert_eq(option_parse(argc, argv), 1);
    cr_assert_eq(option_get_height(), 600);
}
