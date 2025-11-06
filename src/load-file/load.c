#include "load.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



inline size_t length(Substring str) [[unsequenced]] {
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

#if defined(__linux__) || defined(__linux)
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

struct LoadedFile {
  const char* const data;
  const size_t length: SIZE_WIDTH - 1;
  // If true, file is `mmap`ed, and must be `munmap`ed to cleanup.
  // If false, file is `malloc`ed, and must be `free`d to cleanup.
  const bool mapped: 1;
};

struct LoadedFile load_from_filename(const char* const filename) {
  int filedesc = open(filename, O_RDONLY);
  // Set the cursor to end of file and save the current cursor. We don't actually care about the cursor, it's just the easiest way to determine file size.
  size_t len = lseek(filedesc, 0, SEEK_END);
  char* mapping = mmap(NULL, len, PROT_READ, MAP_PRIVATE | MAP_POPULATE, filedesc, 0);
  bool mapped = true;
  if (mapping == NULL) {
    fprintf(stderr, "Failed to memory map file %s. That's really weird, it should basically NEVER happen, so this is probably the fault of whoever last edited this function. No worries, falling back to manually loading it.\n", filename);

    
    mapping = malloc(len);
    if (mapping == NULL) {
      fprintf(stderr, "Well, turns out you're out of memory. Or libc is broken. Either way, *this* failure is not realistically our fault.\n");
      exit(EXIT_FAILURE);
    }
    mapped = false;

    // Reset the cursor to the beginning of the file.
    lseek(filedesc, 0, SEEK_SET);

    size_t bytes_read = 0;

    while (true) {
      ssize_t this_read = read(filedesc, mapping + bytes_read, len - bytes_read);
      if (this_read == -1 && (errno == EAGAIN || errno == EINTR)) {
        continue;
      }
      else if (this_read == -1) {
        fprintf(stderr, "READ CALL FAILURE on file '%s' with descriptor %d after %zu bytes already read with %zu bytes left to read.\n", filename, filedesc, bytes_read, len - bytes_read);
        exit(EXIT_FAILURE);
      }
      else if (this_read == 0 && bytes_read == len) {
        break;
      }
      else if (this_read == 0) {
        fprintf(stderr, "Mysterious end-of-file on file '%s' with descriptor %d at %zu bytes read despite file having been previously measured %zu bytes long. Please do not use this program with files actively being modified.\n", filename, filedesc, bytes_read, len);
        exit(EXIT_FAILURE);
      }
      else {
        bytes_read += this_read;
        if (bytes_read > len) {
          fprintf(stderr, "Mysterious lack of end-of-file on file '%s' with descriptor %d, having reached %zu bytes read despite the file having been previously measured %zu bytes long. Please do not use this program with files actively being modified.\n", filename, filedesc, bytes_read, len);
          exit(EXIT_FAILURE);
        }
      }
    }
    
  }
  struct LoadedFile output = {mapping, len, mapped};
  return output;
}

void unload_file(struct LoadedFile file) {
  if (file.mapped) {
    munmap((void*)file.data, file.length);
  }
  else {
    free((void*)file.data);
  }
}
#endif
