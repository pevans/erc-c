#include <criterion/criterion.h>
#include <unistd.h>

#include "option.h"

static void
setup()
{
    option_set_error("");
}

static void
teardown()
{
}

TestSuite(options, .init = setup, .fini = teardown);

// No need to do this one...
/* Test(option, print_help) */

/* Test(option, get_error) */
/* Test(option, set_error) */
Test(option, error)
{
    char *str = "hahaha FUN";

    cr_assert_str_empty(option_get_error());
    
    option_set_error(str);
    cr_assert_str_eq(option_get_error(), str);
}

Test(option, open_file)
{
    char *str = "so much FUN";
    char *bad_file = "/tmp/BLEH";
    char *file = "/tmp/erc-test.txt";
    char buf[256];
    FILE *stream_a;
    FILE *stream_b;

    // Maybe we should use sterror(ENOENT)?
    cr_assert_eq(option_open_file(&stream_a, bad_file, "r"), 0);
    cr_assert_str_eq(option_get_error(), "open file for /tmp/BLEH: No such file or directory");

    option_set_error("");

    stream_a = fopen(file, "w");
    cr_assert_neq(stream_a, NULL);
    fwrite(str, sizeof(char), strlen(str), stream_a);
    fclose(stream_a);

    option_open_file(&stream_b, file, "r"); 
    fread(buf, sizeof(char), 255, stream_b);
    cr_assert_str_eq(buf, str);
    fclose(stream_b);

    unlink(file);
}

/*
 * This test is really imperfect... that's because option_parse() does
 * a ton of stuff, and is quite complex. I'm punting a lot on the
 * complexity here, while pushing as much of the logic as I can into
 * other functions that are more easily testable.
 */
Test(option, parse)
{
    int argc = 2;
    char *argv[] = {
        "prog_name",
        "--disk1=etc",
    };

    cr_assert_eq(option_parse(argc, argv), 0);
}
