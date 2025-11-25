#include "string-slice.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t length(Substring str)  {
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
struct SubstringArray *new_substring_array(const size_t capacity) {
  struct SubstringArray *output =
      malloc(sizeof(struct SubstringArray) + capacity * sizeof(Substring));

  output->len = 0;
  output->capacity = capacity;

  memset(output->array, 0, capacity);
  return output;
}

struct SubstringArray* append_to_substring_array(struct SubstringArray* array, const Substring str) {
  if (array->len == array->capacity) {
    struct SubstringArray* new_array = realloc(array, sizeof(struct SubstringArray) + array->capacity * 2 * sizeof(Substring));
    
    if (!new_array) {
      fprintf(stderr, "Failed to realloc substring array of length %zu!\n", array->capacity);
      exit(EXIT_FAILURE);
    }
    array = new_array;
  }
  array->array[array->len] = str;
  array->len += 1;
  return array;
}

struct SubstringArray* lines(Substring haystack) {
  struct SubstringArray* output = new_substring_array(0);
  Substring last_line_so_far = haystack;
  last_line_so_far.end = last_line_so_far.start;

  while (last_line_so_far.end < haystack.end) {
    if (*last_line_so_far.end == '\n') {
      output = append_to_substring_array(output, last_line_so_far);
      last_line_so_far.start = last_line_so_far.end + 1;
    }
    last_line_so_far.end++;
  }
  return output;
}


uint32_t fnv1a_hash(Substring str) {
  const uint32_t FNV_OFFSET_BASIS = 0x811c9dc5; // Pulled from https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
  const uint32_t FNV_PRIME = 0x01000193; // Pulled from https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters

  uint32_t hash = FNV_OFFSET_BASIS;

  for (; str.start < str.end; str.start++) {
    hash ^= (uint32_t)(*str.start);
    hash *= FNV_PRIME;
  }

  return hash;
  
}


struct HashArray *new_hash_array(size_t capacity) {
  struct HashArray *output =
      malloc(sizeof(struct HashArray) + capacity * sizeof(uint32_t));

  output->len = 0;
  output->capacity = capacity;

  memset(output->array, 0, capacity);
  return output;
  
}
struct HashArray *new_hash_array_from_substring_array(const struct SubstringArray *str_array) {
  struct HashArray *output =
      malloc(sizeof(struct HashArray) + str_array->len * sizeof(uint32_t));

  output->len = str_array->len;
  output->capacity = str_array->len;

  for (size_t i = 0; i < str_array->len; ++i) {
    output->array[i] = fnv1a_hash(str_array->array[i]);
  }

  return output;
  
}

struct SubstringHashTableEntry {
  Substring str;
  size_t *line_numbers;
  /// Assumes fewer than 4294967296 instances of a given line.
  /// There are other places where we assume fewer than 18446744073709551616
  /// duplicate lines, which are not mentioned because that not being so would imply more
  /// duplicate lines than there are bytes in 64-bit address space, which I
  /// am comfortable assuming isn't going to happen.
  /// More than 4294967296, on the other hand, while wildly unlikely,
  /// is technically possible on ordinary computers such as my laptop, I just doubt it would
  /// ever happen with real code, so I want to record that assumption.
  /// - Lily
  uint32_t line_numbers_len;
  uint32_t hash;
};

struct SubstringHashTable {
  /// Must always be a power of 2
  size_t capacity;
  /// The number of elements actually currently in the table
  size_t load;
  struct SubstringHashTableEntry table[];
};



struct SubstringHashTable *new_substring_hash_table(size_t capacity) {
  struct SubstringHashTable *output =
      malloc(sizeof(struct SubstringHashTable) + capacity * sizeof(struct SubstringHashTableEntry));

  output->load= 0;
  output->capacity = capacity;

  memset(output->table, 0, capacity);
  return output;
  
}


enum LinearProbingAction {
  INITIALIZE_THIS_ENTRY,
  APPEND_TO_THIS_ENTRY,
  PROBE_NEXT_ENTRY,
};


/// `hash` should be the result of calling `fnv1a_hash` on `str`.
/// Tells you what to do next in the linear probing process to insert
/// the substring `str` into this entry.
enum LinearProbingAction check_entry_insert(struct SubstringHashTableEntry *entry, Substring str, uint32_t hash) {
  if (entry->line_numbers == NULL) {
    return INITIALIZE_THIS_ENTRY;
  }
  else if (hash == entry->hash && equal(str, entry->str)) {
    return APPEND_TO_THIS_ENTRY;
  }
  else {
    return PROBE_NEXT_ENTRY;
  }
}


void substring_hash_table_no_realloc_insert(struct SubstringHashTable* table, Substring str, size_t line_number_len, size_t line_numbers[line_number_len]) {
  const uint32_t hash = fnv1a_hash(str);

  if (table->load >= table->capacity / 2) {
    fprintf(stderr, "WARNING: Attempting non-realloc insert to overfull hash table. This is a bad idea, and might infinitely loop.\n");
  }
  

  size_t index = hash & (table->capacity - 1);


  while (true) {
    struct SubstringHashTableEntry* entry = &table->table[index];  

    
    switch (check_entry_insert(entry, str, hash)) {
      case INITIALIZE_THIS_ENTRY:
        entry->hash = hash;
        entry->str = str;
        entry->line_numbers_len = line_number_len;
        entry->line_numbers = malloc(sizeof(size_t) * entry->line_numbers_len);
        memcpy(entry->line_numbers, line_numbers, line_number_len * sizeof(size_t));
        table->load++;
        return;
      case APPEND_TO_THIS_ENTRY:
        entry->line_numbers = realloc(entry->line_numbers, sizeof(size_t) * (entry->line_numbers_len + line_number_len)); // Assume sufficient memory
        memcpy(entry->line_numbers + entry->line_numbers_len, line_numbers, sizeof(size_t) * line_number_len);
        entry->line_numbers_len += line_number_len;
        return;
      case PROBE_NEXT_ENTRY:
        index = (index + 1) & (table->capacity - 1);
        break; // From the switch, not the loop
      // No default case because it is not possible
    }
  }
}

/// Frees the old table, returns a new one with double capacity.
struct SubstringHashTable *reallocate_substring_hash_table(struct SubstringHashTable* old_table) {
  struct SubstringHashTable* new_table = new_substring_hash_table(old_table->capacity * 2);
  size_t entries_moved = 0;
  for (size_t i = 0; i < old_table->capacity && entries_moved < old_table->load; ++i) {
    if (old_table->table[i].line_numbers != NULL) {
      substring_hash_table_no_realloc_insert(new_table, old_table->table[i].str, old_table->table[i].line_numbers_len, old_table->table[i].line_numbers);
    }
  }
  free(old_table);
  return new_table;
}

struct SubstringHashTable *insert_into_substring_hash_table(struct SubstringHashTable* table, Substring str, size_t line_number) {
  if (table->load >= table->capacity / 2) {
    table = reallocate_substring_hash_table(table);
  }

  substring_hash_table_no_realloc_insert(table, str, 1, &line_number);

  return table;
  
}


struct SimpleSizeTArray substring_hash_table_get_entry(struct SubstringHashTable* table, Substring str) {
  const uint32_t hash = fnv1a_hash(str);

  size_t index = hash & (table->capacity - 1);

  struct SimpleSizeTArray output;

  while (true) {
    struct SubstringHashTableEntry* entry = &table->table[index];  

    switch (check_entry_insert(entry, str, hash)) {
      case INITIALIZE_THIS_ENTRY:
        output.array = NULL;
        output.size = 0;
        return output;
      case APPEND_TO_THIS_ENTRY:
        output.array = entry->line_numbers;
        output.size = entry->line_numbers_len;
        return output;
      case PROBE_NEXT_ENTRY:
        index = (index + 1) & (table->capacity - 1);
        break; // From the switch, not the loop
      // No default case because it is not possible
    }
  }
}




