/*
 * objstore.c
 *
 * The code here allows us to work with the object store, which is an
 * entity that allows us to toss all of our binary data needs into a big
 * glob of stuff that we can unpack and use at runtime. Examples of such
 * data are bitmap fonts, ROM data, etc.
 */

#include <zlib.h>

#include "objstore.h"
#include "objstore_data.h"

/*
 * This is data that we expect to find in the store_data variable that
 * is defined in objstore_data.h.
 */
#define HEADER_DATA "hope"

/*
 * Our object store. Just a simple struct that we pull data out from.
 */
static objstore store;

/*
 * This function will set up the store variable so that it contains
 * useful information rather than garbage data from the stack. It does
 * so with a simply memcpy from store_data directly into store, such
 * that the store data itself can be aligned for access.
 */
int
objstore_init()
{
    // Oh, you're calling this again? Cool, but let's bail before we do
    // anything else.
    if (objstore_ready()) {
        return OK;
    }

    // We want to input some bad header data and compare with what
    // eventually should get put into there by memcmp.
    store.header[0] = 'h';
    store.header[1] = 'e';
    store.header[2] = 'y';
    store.header[3] = '\0';

    memcpy(&store, store_data, sizeof(store));

    // If the copy didn't work out somehow...
    if (!objstore_ready()) {
        log_crit("Object store initialization failed with bad data");
        return ERR_BADFILE;
    }

    return OK;
}

/*
 * This will empty out the store, which is not very _practically_
 * useful, but is a bit useful when testing.
 */
void
objstore_clear()
{
    memset(&store, 0, sizeof(store));
}

/*
 * Return true if the object store is ready to be used.
 */
bool
objstore_ready()
{
    int cmp;

    // Test if the header field data is exactly equivalent to that
    // defined in HEADER_DATA. Note we use memcmp(), because the header
    // field is an array of just 4 bytes; strcmp() and strncmp() have an
    // expectation that both pointers will be directed at NUL-terminated
    // strings, which is _not_ the case for store.header (but,
    // incidentally, _is_ the case for HEADER_DATA!).
    cmp = memcmp(store.header, HEADER_DATA,
                 sizeof(store.header) / sizeof(char));

    return cmp == 0;
}

/*
 * Something to simplify the very simple getter functions we use for
 * getting data from the object store.
 */
#define OBJSTORE_DEFN(x) \
    const vm_8bit *objstore_##x() { return store.x; }

/*
 * Note we have an extra semicolon on the end... this is mostly to avoid
 * screwing up my indent, to be honest. But C will ignore the
 * semicolons, so all is well.
 */
OBJSTORE_DEFN(apple2_peripheral_rom);   // ignore docblock
OBJSTORE_DEFN(apple2_sys_rom);          // ignore docblock
OBJSTORE_DEFN(apple2_sysfont);          // ignore docblock
OBJSTORE_DEFN(apple2_invfont);          // ignore docblock
