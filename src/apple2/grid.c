/*
 * apple2.grid.c
 *
 * Each of the display modes in the Apple II work off of a grid, but the
 * grids are generally interleaved, so there's no simple formula to
 * follow.
 *
 * In here we define tables which map addresses to rows or columns
 * within a grid, and some functions to pull that information from those
 * tables.
 */
