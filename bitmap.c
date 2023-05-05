/* A bitmap implementation. */

#include <stdint.h>
#include <stdio.h>

#include "bitmap.h"

#define nth_bit_mask(n) (1 << (n))
#define byte_index(n) ((n) / 8)
#define bit_index(n) ((n) % 8)

// Get the bit at the given index in the bitmap.
int bitmap_get(void *bitmap_start, int index) {
  uint8_t *base = (uint8_t *) bitmap_start;

  return (base[byte_index(index)] >> bit_index(index)) & 1;
}

// Set the bit at the given index in the bitmap to the given value.
void bitmap_put(void *bitmap_start, int index, int value) {
  uint8_t *base = (uint8_t *) bitmap_start;

  long bit_mask = nth_bit_mask(bit_index(index));

  if (value) {
    base[byte_index(index)] |= bit_mask;
  } else {
    bit_mask = ~bit_mask;
    base[byte_index(index)] &= bit_mask;
  }
}

// Pretty-print a given length of bits from a bitmap.
void bitmap_print(void *bitmap_start, int length) {

  for (int i = 0; i < length; i++) {
    putchar(bitmap_get(bitmap_start, i) ? '1' : '0');

    if ((i + 1) % 64 == 0) {
      putchar('\n');
    } else if ((i + 1) % 8 == 0) {
      putchar(' ');
    }
  }
}
