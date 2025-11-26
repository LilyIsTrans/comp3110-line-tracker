#include "../../src/string-slice/string-slice.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

struct SubstringArray* tokenize_lines(struct SubstringArray* lines, char delimiter);
struct SubstringArray* tokenize(Substring haystack, char delimiter);
