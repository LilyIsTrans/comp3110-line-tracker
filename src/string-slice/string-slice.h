#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>

/**
  * Holds a reference to a (POTENTIALLS NON NULL TERMINATED) substring.
  * 
  * This structure contains a reference to some text somewhere in readable memory.
  * The substring pointed to might not be null terminated, so standard string
  * functions are very dangerous and should not be applied.
  *
  * The `end` pointer points to the first address NOT part of this substring (that is,
  * `*(end)` is potentially undefined behaviour, `*(end - 1)` is the last character of
  * the substring).
  */
typedef struct {
  const char* start;
  const char* end;
} Substring;

/**
  * \brief Determines the length (number of `char`s) in a Substring.
  *
  * \param[in] 1 The substring whose length is to be determined.
  */
size_t length(Substring);

/**
  * \brief Compares two substrings to determine if they are exactly equal
  * (that is, contain precisely the same bytes in the same order).
  *
  * \param[in] 1 One substring to be compared
  * \param[in] 2 The other substring to be compared
  */
bool equal(Substring, Substring);


/**
  * \brief Just an array of substrings with tracking for the size.
  */
struct SubstringArray {
  size_t len;
  size_t capacity;
  Substring array[];
};


/**
  * \brief Allocates a new `SubstringArray` with a certain array size.
  * To deallocate the array, just `free` it. This function is a wrapper on
  * `malloc` to keep you from forgetting to add the size of the struct.
  */
struct SubstringArray *new_substring_array(const size_t capacity);

/**
  * \brief Returns an array of substrings from the haystack holding each line in the haystack.
  * Does not include trailing `'\n'` characters, but other whitespace is treated as
  * though it were regular text. Empty lines will correspond to empty substrings, they will
  * not be omitted from the array.
  */
struct SubstringArray* lines(const Substring haystack);



/**
  * \brief Computes the 32-bit FNV-1 hash for a substring.
  */
uint32_t fnv1a_hash(Substring);


/**
  * \brief Just an array of hashes with tracking for the size.
  */
struct HashArray {
  size_t len;
  size_t capacity;
  uint32_t array[];
};


/**
  * \brief Allocates a new `HashArray` with a certain array size.
  * To deallocate the array, just `free` it. This function is a wrapper on
  * `malloc` to keep you from forgetting to add the size of the struct.
  */
struct HashArray *new_hash_array(const size_t capacity);

/**
  * \brief Allocates a new `HashArray` with the same size as the given
  * `SubstringArray` and fills it with the hashes of the corresponding
  * elements of that array. This is just a convenience method, you still
  * need to free the returned array normally and such.
  */
struct HashArray *new_hash_array_from_substring_array(const struct SubstringArray*);



