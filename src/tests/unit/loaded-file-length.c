#include "../../load-file/load.h"
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char** argv) {
  int exit_code = EXIT_SUCCESS;
  for (int i = 1; i < argc - 1; i += 2) {
    struct LoadedFile *file = load_from_filename(argv[i]);
    size_t length = get_size(file);
    size_t expected_length; 
    if (sscanf(argv[i + 1], "%zu", &expected_length) != 1) {
      puts("INVALID COMMAND LINE");
      exit(EXIT_FAILURE);
    }
    if (length != expected_length) {
      exit_code = EXIT_FAILURE;
    }
    printf("File '%s' (number %d) was measured %zu bytes, expected %zu.\n", argv[i], i / 2, length, expected_length);
  }
  return exit_code;
  
}

