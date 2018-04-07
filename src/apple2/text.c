/*
 * apple2.text.c
 *
 * This code sits as a kind of wrapper over the bitmap fonts that the
 * Apple II uses. Neither the system nor inverted fonts are exactly
 * composed in the type of character set that Apple expects; they
 * implement the glpyhs we need in order to make such. You can have
 * inversed characters in the primary character set, for one; in the
 * alternate character set, you can have MouseText glyphs, yet they are
 * implemented as part of both fonts.
 */

#include <ctype.h>

#include "apple2/text.h"

/*
 * This table maps display buffer addresses for 40-column text to the
 * corresponding row on-screen (given a 40x24 grid). A negative column
 * indicates that the address is not displayable.
 */
static int buffer_rows[] = {
//   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,     // $400
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,     // $410
     0,  0,  0,  0,  0,  0,  0,  0,  8,  8,  8,  8,  8,  8,  8,  8,     // $420
     8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,     // $430
     8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,     // $440
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,     // $450
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,     // $460
    16, 16, 16, 16, 16, 16, 16, 16, -1, -1, -1, -1, -1, -1, -1, -1,     // $470
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,     // $480
     1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,     // $490
     1,  1,  1,  1,  1,  1,  1,  1,  9,  9,  9,  9,  9,  9,  9,  9,     // $4A0
     9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,     // $4B0
     9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,     // $4C0
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,     // $4D0
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,     // $4E0
    17, 17, 17, 17, 17, 17, 17, 17, -1, -1, -1, -1, -1, -1, -1, -1,     // $4F0
     2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,     // $500
     2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,     // $510
     2,  2,  2,  2,  2,  2,  2,  2, 10, 10, 10, 10, 10, 10, 10, 10,     // $520
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     // $530
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,     // $540
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,     // $550
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 18,     // $560
    18, 18, 18, 18, 18, 18, 18, 18, -1, -1, -1, -1, -1, -1, -1, -1,     // $570
     3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,     // $580
     3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,     // $590
     3,  3,  3,  3,  3,  3,  3,  3, 11, 11, 11, 11, 11, 11, 11, 11,     // $5A0
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,     // $5B0
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,     // $5C0
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,     // $5D0
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19,     // $5E0
    19, 19, 19, 19, 19, 19, 19, 19, -1, -1, -1, -1, -1, -1, -1, -1,     // $5F0
     4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,     // $600
     4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,     // $610
     4,  4,  4,  4,  4,  4,  4,  4, 12, 12, 12, 12, 12, 12, 12, 12,     // $620
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,     // $630
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,     // $640
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,     // $650
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,     // $660
    20, 20, 20, 20, 20, 20, 20, 20, -1, -1, -1, -1, -1, -1, -1, -1,     // $670
     5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,     // $680
     5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,     // $690
     5,  5,  5,  5,  5,  5,  5,  5, 13, 13, 13, 13, 13, 13, 13, 13,     // $6A0
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,     // $6B0
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,     // $6C0
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,     // $6D0
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,     // $6E0
    21, 21, 21, 21, 21, 21, 21, 21, -1, -1, -1, -1, -1, -1, -1, -1,     // $6F0
     6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,     // $700
     6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,     // $710
     6,  6,  6,  6,  6,  6,  6,  6, 14, 14, 14, 14, 14, 14, 14, 14,     // $720
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,     // $730
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,     // $740
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,     // $750
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,     // $760
    22, 22, 22, 22, 22, 22, 22, 22, -1, -1, -1, -1, -1, -1, -1, -1,     // $770
     7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,     // $780
     7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,     // $790
     7,  7,  7,  7,  7,  7,  7,  7, 15, 15, 15, 15, 15, 15, 15, 15,     // $7A0
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,     // $7B0
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,     // $7C0
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,     // $7D0
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,     // $7E0
    23, 23, 23, 23, 23, 23, 23, 23, -1, -1, -1, -1, -1, -1, -1, -1,     // $7F0
};

/*
 * Similar to buffer_rows, this table defines the column numbers for
 * buffer display addresses.
 */
static int buffer_cols[] = {
//   0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $400
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $410
    32, 33, 34, 35, 36, 37, 38, 39,  0,  1,  2,  3,  4,  5,  6,  7,     // $420
     8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,     // $430
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,     // $440
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $450
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $460
    32, 33, 34, 35, 36, 37, 38, 39, -1, -1, -1, -1, -1, -1, -1, -1,     // $470
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $480
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $490
    32, 33, 34, 35, 36, 37, 38, 39,  0,  1,  2,  3,  4,  5,  6,  7,     // $4A0
     8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,     // $4B0
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,     // $4C0
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $4D0
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $4E0
    32, 33, 34, 35, 36, 37, 38, 39, -1, -1, -1, -1, -1, -1, -1, -1,     // $4F0
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $500
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $510
    32, 33, 34, 35, 36, 37, 38, 39,  0,  1,  2,  3,  4,  5,  6,  7,     // $520
     8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,     // $530
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,     // $540
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $550
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $560
    32, 33, 34, 35, 36, 37, 38, 39, -1, -1, -1, -1, -1, -1, -1, -1,     // $570
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $580
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $590
    32, 33, 34, 35, 36, 37, 38, 39,  0,  1,  2,  3,  4,  5,  6,  7,     // $5A0
     8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,     // $5B0
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,     // $5C0
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $5D0
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $5E0
    32, 33, 34, 35, 36, 37, 38, 39, -1, -1, -1, -1, -1, -1, -1, -1,     // $5F0
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $600
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $610
    32, 33, 34, 35, 36, 37, 38, 39,  0,  1,  2,  3,  4,  5,  6,  7,     // $620
     8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,     // $630
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,     // $640
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $650
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $660
    32, 33, 34, 35, 36, 37, 38, 39, -1, -1, -1, -1, -1, -1, -1, -1,     // $670
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $680
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $690
    32, 33, 34, 35, 36, 37, 38, 39,  0,  1,  2,  3,  4,  5,  6,  7,     // $6A0
     8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,     // $6B0
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,     // $6C0
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $6D0
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $6E0
    32, 33, 34, 35, 36, 37, 38, 39, -1, -1, -1, -1, -1, -1, -1, -1,     // $6F0
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $700
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $710
    32, 33, 34, 35, 36, 37, 38, 39,  0,  1,  2,  3,  4,  5,  6,  7,     // $720
     8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,     // $730
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,     // $740
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $750
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $760
    32, 33, 34, 35, 36, 37, 38, 39, -1, -1, -1, -1, -1, -1, -1, -1,     // $770
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $780
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $790
    32, 33, 34, 35, 36, 37, 38, 39,  0,  1,  2,  3,  4,  5,  6,  7,     // $7A0
     8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,     // $7B0
    24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39,     // $7C0
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,     // $7D0
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,     // $7E0
    32, 33, 34, 35, 36, 37, 38, 39, -1, -1, -1, -1, -1, -1, -1, -1,     // $7F0
};

/*
 * This is the primary character set of display symbols. The order of
 * characters generally follows ASCII order, but doesn't map to ASCII
 * exactly. To begin with, you will note that lower-case characters are
 * not found until the last two rows; moreover, control characters have
 * letter-equivalents which may be displayed--for example, at boot time,
 * when the display buffer is filled with zero-bytes (and the effect is
 * that you see the screen is filled with at symbols).
 */
static char primary_display[] = {
    // $00 - $3F
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', 
    // $40 - $7F
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', 
    // $80 - $BF
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', 
    // $C0 - $FF
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', ' ', 
};

/*
 * Following is the alternate character display set. It differs in some
 * fundamental ways, and particularly it contains "MouseText" symbols
 * (which are not yet implemented!); it also has lower-case symbols in
 * different positions than the primary set.
 */
static char alternate_display[] = {
    // $00 - $3F
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', 
    // $40 - $7F
    // Note that $40 - $5F are actually the MouseText symbols, which are
    // not yet implemented; note also that $60 - $7F are lower-case
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', ' ', 
    // $80 - $BF
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', 
    // $C0 - $FF
    // Note here that, even though lower-case symbols are represented in
    // $60 - $7F, they are repeated here in the same positions as they
    // are in the primary set.
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', ' ', 
};

/*
 * Return the row where the given address will be displayed.
 */
int
apple2_text_row(size_t addr)
{
    return buffer_rows[addr - 0x400];
}

/*
 * Return the column where the given address will be displayed.
 */
int
apple2_text_col(size_t addr)
{
    return buffer_cols[addr - 0x400];
}

/*
 * Return the primary character set representation matching the given
 * character code.
 */
char
apple2_text_primary(vm_8bit ch)
{
    return primary_display[ch];
}

/*
 * Return the alternate character set representation matching the given
 * character code.
 */
char
apple2_text_alternate(vm_8bit ch)
{
    return alternate_display[ch];
}

/*
 * Draw a text character at the given address.
 */
void
apple2_text_draw(apple2 *mach, size_t addr)
{
    int err, row;
    bool inverse, flashing;
    vm_8bit ch;
    vm_area dest;
    vm_bitfont *font;

    // By default, use the primary display set
    char *charset = primary_display;

    // If we're updating a page 2 address and we're not in some kind of
    // double resolution mode, then we shouldn't actually render the
    // thing.
    if (addr > 0x07FF && !apple2_is_double_video(mach)) {
        return;
    }

    // Default
    font = mach->sysfont;

    // This is actually not a byte that is displayable, so let's get out
    row = buffer_rows[addr - 0x400];
    if (row == -1) {
        return;
    }

    err = apple2_text_area(&dest, font, addr);
    if (err != OK) {
        return;
    }

    // What are we working with?
    ch = mos6502_get(mach->cpu, addr);

    // The ASCII code we will use is only that which is composed of the
    // first 6 bits.
    //ch = ch & 0x7f;

    if (mach->display_mode & DISPLAY_ALTCHAR) {
        if (ch < 0x40 || (ch >= 0x60 && ch < 0x7F)) {
            font = mach->invfont;
        } else if (ch >= 0x40 && ch <= 0x5F) {
            font = mach->sysfont;
        }

        charset = alternate_display;
    } else {
        if (ch < 0x40) {
            font = mach->invfont;
        }

        // Note: < 0x80 is flashing, but we don't have that implemented
        // yet
    }

    vm_bitfont_render(font, mach->screen, &dest, charset[ch]);
}

/*
 * This function will fill in the width, height, and x and y offsets for
 * a character at the given address using the given font.
 */
int
apple2_text_area(vm_area *area, vm_bitfont *font, size_t addr)
{
    // The text display buffers are located at "Page 1" and "Page 2",
    // which are at byte 1024-2047 (0x0400-0x07FF) and byte 2048-3071
    // (0x0800-0x0BFF) respectively. If the given address is not in
    // those (contiguous) ranges, then let's bail.
    if (addr < 0x0400 || addr > 0x0BFF) {
        return ERR_INVALID;
    }

    // Certain addresses are not meant to be displayable, and are
    // actually used for system data
    if (buffer_rows[addr] == -1 || buffer_cols[addr] == -1) {
        return ERR_INVALID;
    }

    // In a given page for 40-column mode, you get 960 grid parts that
    // you may use. In 80-column mode, it's more like 1920 grid parts
    // (40x24 = 960, 80x24 = 1920). The way we look at this is the
    // address indicates the place on the grid where text should go. We
    // don't care how it got there. Let's figure out that position
    // on-screen.
    area->xoff = buffer_cols[addr - 0x400] * font->width;
    area->yoff = buffer_rows[addr - 0x400] * font->height;

    // Our width and height must be that of the font.
    area->width = font->width;
    area->height = font->height;

    return OK;
}
