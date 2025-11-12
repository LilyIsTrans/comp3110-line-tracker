#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
/**
  * Holds a reference to a (POTENTIALLS NON NULL TERMINATED) substring.
  * 
  * This structure contains a reference to some text somewhere in readable memory.
  * The substring pointed to might not be null terminated, so standard string
  * functions are very dangerous and should not be applied.
  */
typedef struct {
  const char* const start;
  const char* const end;
} Substring;

/**
  * \brief Determines the length (number of `char`s) in a Substring.
  *
  * \param[in] 1 The substring whose length is to be determined.
  */
inline size_t length(Substring) [[unsequenced]];

/**
  * \brief Compares two substrings to determine if they are exactly equal
  * (that is, contain precisely the same bytes in the same order).
  *
  * \param[in] 1 One substring to be compared
  * \param[in] 2 The other substring to be compared
  */
bool equal(Substring, Substring);

/**
  * Owns the entire contents of a loaded file.
  *
  */
struct LoadedFile;

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
const char* get_data_ptr(struct LoadedFile*) [[unsequenced]];
/**
  * Returns the filesize of the given LoadedFile. 
  */
size_t get_size(struct LoadedFile*) [[unsequenced]];
