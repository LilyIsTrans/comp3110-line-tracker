#include "string-slice.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// length of substring
size_t length(Substring str) { return str.end - str.start; }

// check if substrings are equal
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
  // if equal length, compare string contents
  else {
    return !memcmp(a.start, b.start, len);
  }
}

// create dynamic array to hold substrings
struct SubstringArray *new_substring_array(const size_t capacity) {
  struct SubstringArray *output =
      malloc(sizeof(struct SubstringArray) + capacity * sizeof(Substring));

  output->len = 0;
  output->capacity = capacity;

  // initialize substring array to 0 to prevent garbage values
  if (capacity != 0) {
    memset(output->array, 0, capacity);
  }
  return output;
}

// appends substring to existing array
struct SubstringArray *append_to_substring_array(struct SubstringArray *array,
                                                 const Substring str) {
  // if array is full, reallocate memory for larger array (double capacity)
  if (array->len == array->capacity) {
    struct SubstringArray *new_array =
        realloc(array, sizeof(struct SubstringArray) +
                           array->capacity * 2 * sizeof(Substring));

    // ERROR HANDLING
    if (!new_array) {
      fprintf(stderr, "Failed to realloc substring array of length %zu!\n",
              array->capacity);
      exit(EXIT_FAILURE);
    }
    array = new_array;
    array->capacity *= 2;
  }

  array->array[array->len] = str; // append new substring
  array->len += 1;                // increment length
  return array;
}

// splits substring (haystack) into lines based on '\n' characters
struct SubstringArray *lines(Substring haystack) {
  struct SubstringArray *output =
      new_substring_array(1); // new substring array to hold lines
  Substring last_line_so_far = haystack;
  last_line_so_far.end = last_line_so_far.start;

  // iterate through haystack to find lines
  while (last_line_so_far.end < haystack.end) {
    if (*last_line_so_far.end == '\n') {
      output = append_to_substring_array(
          output,
          last_line_so_far); // append substring up to '\n' to output array
      last_line_so_far.start =
          last_line_so_far.end +
          1; // update start pointer to character after '\n'
    }
    last_line_so_far.end++;
  }
  return output;
}

// calculates FNV-1a hash for substrings
uint32_t fnv1a_hash(Substring str) {
  const uint32_t FNV_OFFSET_BASIS =
      0x811c9dc5; // Pulled from
                  // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
  const uint32_t FNV_PRIME =
      0x01000193; // Pulled from
                  // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters

  uint32_t hash = FNV_OFFSET_BASIS;

  for (; str.start < str.end; str.start++) {
    hash ^= (uint32_t)(*str.start);
    hash *= FNV_PRIME;
  }

  return hash;
}
// calculates FNV-1 hash for substrings
uint32_t fnv1_hash(Substring str) {
  const uint32_t FNV_OFFSET_BASIS =
      0x811c9dc5; // Pulled from
                  // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters
  const uint32_t FNV_PRIME =
      0x01000193; // Pulled from
                  // https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function#FNV_hash_parameters

  uint32_t hash = FNV_OFFSET_BASIS;

  for (; str.start < str.end; str.start++) {
    hash *= FNV_PRIME;
    hash ^= (uint32_t)(*str.start);
  }

  return hash;
}

// allocate memory for HashArray struct
struct HashArray *new_hash_array(size_t capacity) {
  struct HashArray *output =
      malloc(sizeof(struct HashArray) + capacity * sizeof(uint32_t));

  output->len = 0;
  output->capacity = capacity;

  memset(output->array, 0, capacity);
  return output;
}

// allocate memeory for HashArray from SubstringArray and fill with hashes
struct HashArray *
new_hash_array_from_substring_array(const struct SubstringArray *str_array) {
  struct HashArray *output =
      malloc(sizeof(struct HashArray) + str_array->len * sizeof(uint32_t));

  output->len = str_array->len;
  output->capacity = str_array->len;

  for (size_t i = 0; i < str_array->len; ++i) {
    output->array[i] = fnv1a_hash(str_array->array[i]);
  }

  return output;
}

// define an entry in hash table
struct SubstringHashTableEntry {
  Substring str;
  size_t *line_numbers;
  /// Assumes fewer than 4294967296 instances of a given line.
  /// There are other places where we assume fewer than 18446744073709551616
  /// duplicate lines, which are not mentioned because that not being so would
  /// imply more duplicate lines than there are bytes in 64-bit address space,
  /// which I am comfortable assuming isn't going to happen. More than
  /// 4294967296, on the other hand, while wildly unlikely, is technically
  /// possible on ordinary computers such as my laptop, I just doubt it would
  /// ever happen with real code, so I want to record that assumption.
  /// - Lily
  uint32_t line_numbers_len;
  uint32_t hash;
};

// define the layout of hash table
struct SubstringHashTable {
  /// Must always be a power of 2
  size_t capacity;
  /// The number of elements actually currently in the table
  size_t load;
  struct SubstringHashTableEntry table[];
};

// allocate memory for SubstringHashTable struct
struct SubstringHashTable *new_substring_hash_table(size_t capacity) {
  struct SubstringHashTable *output =
      malloc(sizeof(struct SubstringHashTable) +
             capacity * sizeof(struct SubstringHashTableEntry));

  output->load = 0;
  output->capacity = capacity;

  memset(output->table, 0, capacity);
  return output;
}

void free_substring_hash_table(struct SubstringHashTable *table) {
  for (size_t i = 0; i < table->capacity; ++i) {
    free(table->table[i].line_numbers);
  }
  free(table);
}

// define actions for probing (resolving collisions in hash table)
enum ProbingAction {
  INITIALIZE_THIS_ENTRY, // current entry is empty, initialize it
  APPEND_TO_THIS_ENTRY,  // current entry matches, append line number
  PROBE_NEXT_ENTRY, // current entry occupied but does not match, probe next
                    // entry
};

/// `hash` should be the result of calling `fnv1a_hash` on `str`.
/// Tells you what to do next in the linear probing process to insert
/// the substring `str` into this entry.
enum ProbingAction check_entry(struct SubstringHashTableEntry *entry,
                               Substring str, uint32_t hash) {
  // initialize entry if empty
  if (entry->line_numbers == NULL) {
    return INITIALIZE_THIS_ENTRY;
  }
  // append new line number if hash and substring match
  else if (hash == entry->hash && equal(str, entry->str)) {
    return APPEND_TO_THIS_ENTRY;
  }
  // check next hash table entry
  else {
    return PROBE_NEXT_ENTRY;
  }
}

// insert substring into hash table without reallocating (assume sufficient
// memory is available)
//  Takes ownership of `line_numbers`
void substring_hash_table_no_realloc_insert(struct SubstringHashTable *table,
                                            Substring str,
                                            size_t line_number_len,
                                            size_t *line_numbers) {
  const uint32_t hash = fnv1a_hash(str);

  // if table is over half full, print warning
  if (table->load >= table->capacity / 2) {
    fprintf(stderr, "WARNING: Attempting non-realloc insert to overfull hash "
                    "table. This is a bad idea, and might infinitely loop.\n");
  }

  size_t index = hash & (table->capacity - 1);
  const uint32_t step_size = (fnv1_hash(str) & (table->capacity - 1)) | 1;

  // linear probing to find appropriate entry
  while (true) {
    struct SubstringHashTableEntry *entry = &table->table[index];

    switch (check_entry(entry, str, hash)) {

    case INITIALIZE_THIS_ENTRY:
      entry->hash = hash;
      entry->str = str;
      entry->line_numbers_len = line_number_len;
      entry->line_numbers = line_numbers;
      table->load++;
      return;

    case APPEND_TO_THIS_ENTRY:
      entry->line_numbers = realloc(
          entry->line_numbers,
          sizeof(size_t) * (entry->line_numbers_len +
                            line_number_len)); // Assume sufficient memory
      memcpy(entry->line_numbers + entry->line_numbers_len, line_numbers,
             sizeof(size_t) * line_number_len);
      free(line_numbers);
      entry->line_numbers_len += line_number_len;
      return;

    case PROBE_NEXT_ENTRY:
      index = (index + step_size) & (table->capacity - 1);
      break; // From the switch, not the loop
      // No default case because it is not possible
    }
  }
}

/// Frees the old table, returns a new one with double capacity.
struct SubstringHashTable *
reallocate_substring_hash_table(struct SubstringHashTable *old_table) {
  struct SubstringHashTable *new_table =
      new_substring_hash_table(old_table->capacity * 2);
  size_t entries_moved = 0;

  // copy entries from old table to new table
  for (size_t i = 0; i < old_table->capacity && entries_moved < old_table->load;
       ++i) {
    if (old_table->table[i].line_numbers != NULL) {
      substring_hash_table_no_realloc_insert(
          new_table, old_table->table[i].str,
          old_table->table[i].line_numbers_len,
          old_table->table[i].line_numbers);
    }
  }

  free(old_table); // `substring_hash_table_no_realloc_insert` takes ownership
                   // of all the internal pointers, so this is the correct
  // action, not the more complicated hash table free (which would actually be
  // undefined behaviour).
  return new_table;
}

// reallocate hash table if necessary (more than half full)
struct SubstringHashTable *
insert_into_substring_hash_table(struct SubstringHashTable *table,
                                 Substring str, size_t *line_number) {
  if (table->load >= table->capacity / 4) {
    table = reallocate_substring_hash_table(table);
  }

  substring_hash_table_no_realloc_insert(table, str, 1, line_number);

  return table;
}

// get line numbers for given substring from hash table
struct SimpleSizeTArray
substring_hash_table_get_entry(struct SubstringHashTable *table,
                               Substring str) {
  const uint32_t hash = fnv1a_hash(str);

  const size_t canonical_index = hash & (table->capacity - 1);
  const uint32_t step_size = (fnv1_hash(str) & (table->capacity - 1)) | 1;
  size_t index = canonical_index;

  struct SimpleSizeTArray output;

  while (true) {
    struct SubstringHashTableEntry *entry = &table->table[index];

    switch (check_entry(entry, str, hash)) {
    case PROBE_NEXT_ENTRY:
      index = (index + step_size) & (table->capacity - 1);
      if (index != canonical_index) {
        break; // From the switch, not the loop, actually restarts the loop
      }
      // Intentional fallthrough
    case INITIALIZE_THIS_ENTRY:
      output.array = NULL;
      output.size = 0;
      return output;
    case APPEND_TO_THIS_ENTRY:
      output.array = entry->line_numbers;
      output.size = entry->line_numbers_len;
      return output;
      // No default case because it is not possible
    }
  }
}

// get info about hash table performance
struct HashTablePerformanceHeuristics *
get_substring_hash_table_performance_heuristics(
    struct SubstringHashTable *table) {
  struct HashTablePerformanceHeuristics *output =
      malloc(sizeof(struct HashTablePerformanceHeuristics));

  output->capacity = table->capacity; // max num of entries table can hold
  output->load = table->load;         // num of entries currently in table
  output->clusters = 0;               // num of clusters of entries
  output->entries_not_at_home = 0;    // num of entries not at expected position
  output->max_cluster_size = 0;       // size of largest cluster

  bool currently_in_cluster = false;
  size_t current_cluster_size = 0;

  // calculate cluster info
  for (size_t i = 0; i < table->capacity; ++i) {
    if (currently_in_cluster && table->table[i].line_numbers == NULL) {
      currently_in_cluster = false;

      if (current_cluster_size > output->max_cluster_size) {
        output = realloc(output, sizeof(struct HashTablePerformanceHeuristics) +
                                     current_cluster_size * sizeof(size_t));
        memset(output->cluster_size_populations + output->max_cluster_size, 0,
               sizeof(size_t) *
                   (current_cluster_size - output->max_cluster_size));
        output->max_cluster_size = current_cluster_size;
      }

      output->cluster_size_populations[current_cluster_size - 1]++;
      current_cluster_size = 0;
    } else if (currently_in_cluster) {
      current_cluster_size++;
      output->entries_not_at_home++;
    } else if (table->table[i].line_numbers != NULL) {
      output->clusters++;
      current_cluster_size++;
      currently_in_cluster = true;
    }
  }
  if (currently_in_cluster) {
    output->max_cluster_size = current_cluster_size > output->max_cluster_size
                                   ? current_cluster_size
                                   : output->max_cluster_size;
  }

  return output;
}

// `lines` and `strings` have identical length and capacity and corresponding
// entries; any modification to one should have a corresponding modification to
// the other to maintain synchronicity.
struct SubstringWithOriginLineArray {
  size_t len;
  size_t capacity;
  size_t *lines;
  Substring *strings;
};

// Allocates the underlying arrays in a `SubstringWithOriginLineArray`
void malloc_substring_with_origin_line_array(
    struct SubstringWithOriginLineArray *arr, size_t size) {
  arr->lines = malloc(sizeof(size_t) * size);
  arr->strings = malloc(sizeof(Substring) * size);
  arr->capacity = size;
  arr->len = 0;
  if (arr->lines == NULL || arr->strings == NULL) {
    exit(EXIT_FAILURE);
  }
}

// Reallocates the underlying arrays in a `SubstringWithOriginLineArray`. May
// invalidate the old pointers. Also updates the length and capacity
// bookkeeping.
void realloc_substring_with_origin_line_array(
    struct SubstringWithOriginLineArray *arr, size_t new_size) {
  arr->lines = realloc(arr->lines, sizeof(size_t) * new_size);
  arr->strings = realloc(arr->strings, sizeof(Substring) * new_size);
  arr->capacity = new_size;
  arr->len = arr->len > new_size ? new_size : arr->len;
  if (arr->lines == NULL || arr->strings == NULL) {
    exit(EXIT_FAILURE);
  }
}

// Frees the underlying array of a `SubstringWithOriginLineArray`. DOES NOT free
// the actual passed pointer, as it's perfectly valid for that to live on the
// stack; if it's heap allocated, the caller must free it themselves.
void free_substring_with_origin_line_array(
    struct SubstringWithOriginLineArray *arr) {
  free(arr->lines);
  free(arr->strings);
}

/// Returns an array of lines from `old_file_lines` which were found nowhere in
/// `new_file_lines`
struct SubstringWithOriginLineArray 
unmatched_lines(struct SubstringArray *old_file_lines,
                struct SubstringArray *new_file_lines) {
  size_t hash_table_size = new_file_lines->len;
  {
    size_t mask =
        ~((size_t)(-1) >>
          1); // Set mask to have a single 1 bit in the most significant
              // position followed by however many trailing zeroes are necessary
    while ((mask & hash_table_size) == 0) {
      mask >>= 1;
    }
    hash_table_size = mask << 1;
  };

  struct SubstringHashTable *new_lines_table =
      new_substring_hash_table(hash_table_size);

  for (size_t i = 0; i < new_file_lines->len; ++i) {
    if (new_file_lines->array[i].end != new_file_lines->array[i].start) {
      size_t *I = malloc(sizeof(i));
      *I = i;
      new_lines_table = insert_into_substring_hash_table(
          new_lines_table, new_file_lines->array[i], I);
    }
  }


  struct SubstringWithOriginLineArray output;
  malloc_substring_with_origin_line_array(&output, old_file_lines->len);

  for (size_t i = 0; i < old_file_lines->len; ++i) {
    if (substring_hash_table_get_entry(new_lines_table, old_file_lines->array[i]).size == 0) {
      if (output.len >= output.capacity) {
        realloc_substring_with_origin_line_array(&output, output.capacity * 2);
      }
      output.lines[output.len] = i;
      output.strings[output.len] = old_file_lines->array[i];
      output.len++;
    }
  }
  if (output.len != output.capacity) {
    realloc_substring_with_origin_line_array(&output, output.len);
  }

  return output;
}
size_t substring_with_origin_line_array_get_length(struct SubstringWithOriginLineArray* arr) {
  return arr->len;
}
size_t substring_with_origin_line_array_get_origin_line(struct SubstringWithOriginLineArray* arr, size_t index) {
  return arr->lines[index];
}
Substring substring_with_origin_line_array_get_substring(struct SubstringWithOriginLineArray* arr, size_t index) {
  return arr->strings[index];
}
