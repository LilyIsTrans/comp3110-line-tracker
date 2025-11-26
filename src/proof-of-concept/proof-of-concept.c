#include "../../src/load-file/load.h"
#include "../../src/string-slice/string-slice.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Expected 2 arguments, found %d.\n", argc - 1);
    return EXIT_FAILURE;
  } else if (argc > 3) {
    fprintf(stderr, "%d unexpected arguments, ignoring.\n", argc - 3);
  }

  struct LoadedFile *prior_version = load_from_filename(argv[1]);
  struct LoadedFile *new_version = load_from_filename(argv[2]);

  Substring prior_content;
  prior_content.start = get_data_ptr(prior_version);
  prior_content.end = prior_content.start + get_size(prior_version);

  Substring new_content;
  new_content.start = get_data_ptr(new_version);
  new_content.end = new_content.start + get_size(new_version);

  struct SubstringArray *prior_lines = lines(prior_content);
  struct SubstringArray *new_lines = lines(new_content);

  size_t hash_table_size = new_lines->len;
  {
    size_t mask = ~((size_t)(-1) >> 1); // Set mask to have a single 1 bit in the most significant position followed by however many trailing zeroes are necessary
    while ((mask & hash_table_size) == 0) {
      mask >>= 1;
    }
    hash_table_size = mask << 1;
  }

  struct SubstringHashTable* new_lines_table = new_substring_hash_table(hash_table_size);

  for (size_t i = 0; i < new_lines->len; ++i) {
    insert_into_substring_hash_table(new_lines_table, new_lines->array[i], i);
  }

  for (size_t i = 0; i < prior_lines->len; ++i) {
    struct SimpleSizeTArray lines_found = substring_hash_table_get_entry(new_lines_table, prior_lines->array[i]);
    printf("Line %zu in '%s' was found on lines: ", i + 1, argv[1]);
    for (size_t j = 0; j < lines_found.size; ++j) {
      printf("%zu", lines_found.array[j] + 1);
      if (j < lines_found.size - 1) {
        printf(", ");
      }
    }
    printf(" in '%s'\n", argv[2]);
  }
}
