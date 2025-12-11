#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/**
  * Owns the entire contents of a loaded file.
  *
  */
struct LoadedFile {
  const char* data;
  size_t length: sizeof(size_t) * 8 - 1;
  // If true, file is `mmap`ed, and must be `munmap`ed to cleanup.
  // If false, file is `malloc`ed, and must be `free`d to cleanup.
  bool mapped: 1;
};
/**
  * Attempts to create a new LoadedFile from the given filename.
  * Makes every attempt to do as little work as possible in this process.
  * If the `data` member of the returned LoadedFile is `NULL`, and the `length`
  * member is `0`, then the file was an empty file (and no memory was allocated).
  */
struct LoadedFile* load_from_filename(const char* const filename);

/**
  * Frees up whatever resources backed the `LoadedFile`. After this function is called, the passed pointer to `LoadedFile` is invalid and must not be used.
  */
void unload_file(struct LoadedFile*);

/**
  * Returns the `data` pointer of the given LoadedFile. 
  */
const char* get_data_ptr(struct LoadedFile*) ;
/**
  * Returns the filesize of the given LoadedFile. 
  */
size_t get_size(struct LoadedFile*) ;
