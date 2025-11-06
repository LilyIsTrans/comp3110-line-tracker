#pragma once
#include <stdlib.h>

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
  * \param The substring whose length is to be determined.
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
