#include "string-slice.h"

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
