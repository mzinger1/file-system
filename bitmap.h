/* A bitmap interface. */

#ifndef BITMAP_H
#define BITMAP_H

// Get the bit at the given index in the bitmap.
int bitmap_get(void *bitmap_start, int index);

// Set the bit at the given index in the bitmap to the given value.
void bitmap_put(void *bitmap_start, int index, int value);

// Pretty-print a given length of bits from a bitmap.
void bitmap_print(void *bitmap_start, int length);

#endif
