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
  const char* data;
  size_t length: sizeof(size_t) * 8 - 1;
  // If true, file is `mmap`ed, and must be `munmap`ed to cleanup.
  // If false, file is `malloc`ed, and must be `free`d to cleanup.
  bool mapped: 1;
};

struct LoadedFile* load_from_filename(const char* const filename) {
  int filedesc = open(filename, O_RDONLY);
  // Set the cursor to end of file and save the current cursor. We don't actually care about the cursor, it's just the easiest way to determine file size.
  size_t len = lseek(filedesc, 0, SEEK_END);
  char* mapping = mmap(NULL, len, PROT_READ, MAP_PRIVATE | MAP_POPULATE, filedesc, 0);
  bool mapped = true;
  if (mapping == NULL) {
    fprintf(stderr, "Failed to memory map file %s. That's really weird, it should basically NEVER happen, so this is probably the fault of whoever last edited this function. No worries, falling back to manually loading it. The error reporting mechanism says '%s'.\n", filename, strerror(errno));

    
    mapping = malloc(len);
    if (mapping == NULL) {
      fprintf(stderr, "Well, turns out you're out of memory. Or libc is broken. Either way, *this* failure is not realistically our fault. The error was '%s'.\n", strerror(errno));
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
        fprintf(stderr, "READ CALL FAILURE on file '%s' with descriptor %d after %zu bytes already read with %zu bytes left to read. The error was '%s'.\n", filename, filedesc, bytes_read, len - bytes_read, strerror(errno));
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
  struct LoadedFile output_data = {mapping, len, mapped};
  struct LoadedFile *output = malloc(sizeof(struct LoadedFile));
  *output = output_data;
  return output;
}

void unload_file(struct LoadedFile *file) {
  if (file->mapped) {
    munmap((void*)file->data, file->length);
    free(file);
  }
  else {
    free((void*)file->data);
    free(file);
  }
}

#else
#include <stdio.h>
#include <errno.h>


struct LoadedFile {
  const char* data;
  size_t length;
};
struct LoadedFile* load_from_filename(const char* const filename) {
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Failed to open file '%s', reason: '%s'.\n", filename, strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (!fseek(file, 0, SEEK_END)) {
    fprintf(stderr, "Failed to seek to end of file '%s'. Either your libc is broken or your computer is haunted. Officially, the error was '%s'.\n", filename, strerror(errno));
    exit(EXIT_FAILURE);
  }
  int filesize_temp = ftell(file);
  if (filesize_temp == -1) {
    fprintf(stderr, "Failed to read cursor position for file '%s'. Either your libc is broken or your computer is haunted. Officially, the error was '%s'.\n", filename, strerror(errno));
    exit(EXIT_FAILURE);
  }
  size_t filesize = filesize_temp;

  char* data = malloc(filesize);
  if (data == NULL) {
    fprintf(stderr, "Failed to allocate %zu bytes to store the data of file '%s'. Reason: %s.\n", filesize, filename, strerror(errno));
    exit(EXIT_FAILURE);
  }

  size_t bytes_read = 0;

  while (true) {
      size_t this_read = fread(data + bytes_read, 1, filesize - bytes_read, file);
      if (ferror(file)) {
        fprintf(stderr, "Failed to read from file '%s' at position %zu with %zu bytes to go because '%s'.\n", filename, bytes_read, filesize - bytes_read, strerror(ferror(file)));
        exit(EXIT_FAILURE);
      }
      else if (feof(file) && bytes_read == filesize) {
        break;
      }
      else if (feof(file)) {
        fprintf(stderr, "Mysterious end-of-file on file '%s' at %zu bytes read despite file having been previously measured %zu bytes long. Please do not use this program with files actively being modified.\n", filename, bytes_read, filesize);
        exit(EXIT_FAILURE);
      }
      else {
        bytes_read += this_read;
        if (bytes_read > filesize) {
          fprintf(stderr, "Mysterious lack of end-of-file on file '%s', having reached %zu bytes read despite the file having been previously measured %zu bytes long. Please do not use this program with files actively being modified.\n", filename, bytes_read, filesize);
          exit(EXIT_FAILURE);
        }
      }
    
  }

  fclose(file);

  struct LoadedFile output_data = {data, filesize};
  struct LoadedFile *output = malloc(sizeof(struct LoadedFile));
  *output = output_data;
  return output;
  
}

void unload_file(struct LoadedFile *file) {
  free((void*)file->data);
  free(file);
}

#endif

const char* get_data_ptr(struct LoadedFile* file) [[unsequenced]] {
  return file->data;
}
size_t get_size(struct LoadedFile* file) [[unsequenced]] {
  return file->length;
}
