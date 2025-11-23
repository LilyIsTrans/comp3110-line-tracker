#include <stdlib.h>
#include <stdbool.h>


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
  const char* const start;
  const char* const end;
} Substring;

/**
  * \brief Determines the length (number of `char`s) in a Substring.
  *
  * \param[in] 1 The substring whose length is to be determined.
  */
inline size_t length(Substring);

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
  Substring array[];
};


/**
  * \brief Allocates a new `SubstringArray` with a certain array size.
  * To deallocate the array, just `free` it. This function is a thin wrapper on
  * `malloc` to keep you from forgetting to add the size of the struct.
  */
inline struct SubstringArray* new_substring_array(const size_t size) {
  return malloc(sizeof(struct SubstringArray) + size * sizeof(Substring));
}


/**
  * \brief Returns an array of substrings from the haystack holding each line in the haystack.
  * Does not include trailing `'\n'` or `'\r'` characters, but other whitespace is treated as
  * though it were regular text. Empty lines will correspond to empty substrings, they will
  * not be omitted from the array.
  */
struct SubstringArray* lines(const Substring haystack);


