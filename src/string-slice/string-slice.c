#include "string-slice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t length(Substring str)  {
  return str.end - str.start;
}


bool equal(Substring a, Substring b) {
  size_t len;

  if (a.start == b.start && a.end == b.end) {
    return true;
  }
  // Use of the result of assignment as an operand to comparison intentional,
  // this is a rare case where that's actually the most elegant way to write
  // this.
  else if ((len = length(a) != length(b))) {
    return false;
  }
  else {
    return !memcmp(a.start, b.start, len);
  }
}
struct SubstringArray *new_substring_array(const size_t capacity) {
  struct SubstringArray *output =
      malloc(sizeof(struct SubstringArray) + capacity * sizeof(Substring));

  output->len = 0;
  output->capacity = capacity;

  memset(output->array, 0, capacity);
  return output;
}

struct SubstringArray* append_to_substring_array(struct SubstringArray* array, const Substring str) {
  if (array->len == array->capacity) {
    struct SubstringArray* new_array = realloc(array, sizeof(struct SubstringArray) + array->capacity * 2 * sizeof(Substring));
    
    if (!new_array) {
      fprintf(stderr, "Failed to realloc substring array of length %zu!\n", array->capacity);
      exit(EXIT_FAILURE);
    }
    array = new_array;
  }
  array->array[array->len] = str;
  array->len += 1;
  return array;
}

struct SubstringArray* lines(Substring haystack) {
  struct SubstringArray* output = new_substring_array(0);
  Substring last_line_so_far = haystack;
  last_line_so_far.end = last_line_so_far.start;

  while (last_line_so_far.end < haystack.end) {
    if (*last_line_so_far.end == '\n') {
      output = append_to_substring_array(output, last_line_so_far);
      last_line_so_far.start = last_line_so_far.end + 1;
    }
    last_line_so_far.end++;
  }
  return output;
}
