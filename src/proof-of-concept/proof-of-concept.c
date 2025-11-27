#include "../../src/load-file/load.h"
#include "../../src/string-slice/string-slice.h"
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#define atomic _Atomic
struct output_buffer {
  char *write_cursor;
  cnd_t wait_for_data;
  mtx_t wait_for_data_mtx;
  atomic bool mutex_ready;
  atomic bool exit_thread;
  char buf[1 << 14];
};

static struct output_buffer out_bufs[2];

void init_output_buffers() {
  memset(out_bufs, 0, sizeof(out_bufs));
  cnd_init(&out_bufs[0].wait_for_data);
  cnd_init(&out_bufs[1].wait_for_data);
  mtx_init(&out_bufs[0].wait_for_data_mtx, mtx_plain);
  mtx_init(&out_bufs[1].wait_for_data_mtx, mtx_plain);
  out_bufs[0].write_cursor = out_bufs[0].buf;
  out_bufs[1].write_cursor = out_bufs[1].buf;
  atomic_store_explicit(&out_bufs[0].mutex_ready, false, memory_order_relaxed);
  atomic_store_explicit(&out_bufs[1].mutex_ready, false, memory_order_relaxed);
  atomic_store_explicit(&out_bufs[0].exit_thread, false, memory_order_relaxed);
  atomic_store_explicit(&out_bufs[1].exit_thread, false, memory_order_relaxed);
}

int printing_thread(void *output_buffer_) {

  struct output_buffer *output_buffer =
      (struct output_buffer *)(output_buffer_);
  mtx_lock(&output_buffer->wait_for_data_mtx);
  atomic_store_explicit(&output_buffer->mutex_ready, true,
                        memory_order_release);

  while (true) {
    cnd_wait(&output_buffer->wait_for_data, &output_buffer->wait_for_data_mtx);
    if (atomic_load_explicit(&output_buffer->exit_thread, memory_order_acquire)) {
      thrd_exit(0);
    }
    fputs(output_buffer->buf, stdout);
    memset(output_buffer->buf, 0, sizeof(output_buffer->buf));
    output_buffer->write_cursor = output_buffer->buf;
  }
}

void async_print(int *current_buffer, const char *format, ...) {
  va_list var_args, backup_args;
  va_start(var_args, format);
  va_copy(backup_args, var_args);
  mtx_lock(&out_bufs[*current_buffer].wait_for_data_mtx);
  size_t available_space =
      (out_bufs[*current_buffer].buf + sizeof(out_bufs[*current_buffer].buf)) -
      out_bufs[*current_buffer].write_cursor;
  size_t printed = vsnprintf(out_bufs[*current_buffer].write_cursor,
                             available_space, format, var_args);
  if (printed >= available_space) {
    *out_bufs[*current_buffer].write_cursor = '\0';
    cnd_signal(&out_bufs[*current_buffer].wait_for_data);
    mtx_unlock(&out_bufs[*current_buffer].wait_for_data_mtx);
    *current_buffer ^= 1;
    mtx_lock(&out_bufs[*current_buffer].wait_for_data_mtx);
    size_t available_space = (out_bufs[*current_buffer].buf +
                              sizeof(out_bufs[*current_buffer].buf)) -
                             out_bufs[*current_buffer].write_cursor;
    printed = vsnprintf(out_bufs[*current_buffer].write_cursor, available_space,
                        format, backup_args);
    mtx_unlock(&out_bufs[*current_buffer].wait_for_data_mtx);

  } else {
    out_bufs[*current_buffer].write_cursor += printed;
    mtx_unlock(&out_bufs[*current_buffer].wait_for_data_mtx);
  }
  va_end(var_args);
  va_end(backup_args);
}

int main(int argc, const char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Expected 2 arguments, found %d.\n", argc - 1);
    return EXIT_FAILURE;
  } else if (argc > 3) {
    fprintf(stderr, "%d unexpected arguments, ignoring.\n", argc - 3);
  }

  struct LoadedFile *prior_version = load_from_filename(argv[1]);
  struct LoadedFile *new_version = load_from_filename(argv[2]);

  Substring prior_content;
  prior_content.start = get_data_ptr(prior_version);
  prior_content.end = prior_content.start + get_size(prior_version);

  Substring new_content;
  new_content.start = get_data_ptr(new_version);
  new_content.end = new_content.start + get_size(new_version);

  struct SubstringArray *prior_lines = lines(prior_content);
  struct SubstringArray *new_lines = lines(new_content);

  size_t hash_table_size = new_lines->len;
  {
    size_t mask =
        ~((size_t)(-1) >>
          1); // Set mask to have a single 1 bit in the most significant
              // position followed by however many trailing zeroes are necessary
    while ((mask & hash_table_size) == 0) {
      mask >>= 1;
    }
    hash_table_size = mask << 1;
  }

  struct SubstringHashTable *new_lines_table =
      new_substring_hash_table(hash_table_size);

  for (size_t i = 0; i < new_lines->len; ++i) {
    if (new_lines->array[i].end != new_lines->array[i].start) {
      size_t* I = malloc(sizeof(i));
      *I = i;
      new_lines_table = insert_into_substring_hash_table(
          new_lines_table, new_lines->array[i], I);
    }
  }

  // struct HashTablePerformanceHeuristics *perf =
  // get_substring_hash_table_performance_heuristics(new_lines_table);

  // printf("Capacity: %zu\nLoad: %zu\nEntries Not In Optimal Location:
  // %zu\nNumber of clusters: %zu\nLargest cluster size: %zu\nCluster Size:",
  // perf->capacity, perf->load, perf->entries_not_at_home, perf->clusters,
  // perf->max_cluster_size); for (size_t i = 0; i < perf->max_cluster_size;
  // ++i) {
  //   printf("\t%zu", i + 1);
  // }
  // printf("\nClusters:");
  // for (size_t i = 0; i < perf->max_cluster_size; ++i) {
  //   printf("\t%zu", perf->cluster_size_populations[i]);
  // }
  // putchar('\n');

  int current_buffer = 0;

  init_output_buffers();
  thrd_t output_threads[2];
  thrd_create(&output_threads[0], printing_thread, &out_bufs[0]);
  thrd_create(&output_threads[1], printing_thread, &out_bufs[1]);
  while (
      !atomic_load_explicit(&out_bufs[0].mutex_ready, memory_order_acquire) ||
      !atomic_load_explicit(&out_bufs[1].mutex_ready, memory_order_acquire)) {
    // Intentionally empty, we're just stalling
  }

  // for (size_t i = 0; i < prior_lines->len; ++i) {
  //   struct SimpleSizeTArray lines_found =
  //       substring_hash_table_get_entry(new_lines_table,
  //       prior_lines->array[i]);

  //   async_print(&current_buffer, "Line %zu in '%s' was found on lines: ", i +
  //   1, argv[1]); for (size_t j = 0; j < lines_found.size; ++j) {
  //     async_print(&current_buffer, "%zu", lines_found.array[j] + 1);
  //     if (j < lines_found.size - 1) {
  //       async_print(&current_buffer, ", ");
  //     }
  //   }
  //   async_print(&current_buffer, " in '%s'\n", argv[2]);
  // }

  struct SimpleSizeTArray *duplicates_of = malloc(prior_lines->len * sizeof(struct SimpleSizeTArray));
  for (size_t i = 0; i < prior_lines->len; ++i) {
    duplicates_of[i] =
        substring_hash_table_get_entry(new_lines_table, prior_lines->array[i]);
  }


  for (size_t i = 0; i < prior_lines->len; ++i) {
    if (duplicates_of[i].size != 0) {
      async_print(&current_buffer, "Line %zu in '%s' was found on lines: ", i + 1,
                  argv[1]);
      for (size_t j = 0; j + 1 < duplicates_of[i].size; ++j) {
        async_print(&current_buffer, "%zu, ", duplicates_of[i].array[j] + 1);
      }
      async_print(&current_buffer, "%zu in '%s'\n",
                  duplicates_of[i].array[duplicates_of[i].size - 1], argv[2]);
      
    }
  }
  free(duplicates_of);
  atomic_store_explicit(&out_bufs[0].exit_thread, true, memory_order_release);
  atomic_store_explicit(&out_bufs[1].exit_thread, true, memory_order_release);
  cnd_signal(&out_bufs[0].wait_for_data);
  cnd_signal(&out_bufs[1].wait_for_data);
  free_substring_hash_table(new_lines_table);
  thrd_join(output_threads[0], NULL);
  thrd_join(output_threads[1], NULL);
  cnd_destroy(&out_bufs[0].wait_for_data);
  cnd_destroy(&out_bufs[1].wait_for_data);
  mtx_destroy(&out_bufs[0].wait_for_data_mtx);
  mtx_destroy(&out_bufs[1].wait_for_data_mtx);

  unload_file(prior_version);
  unload_file(new_version);

  free(prior_lines);
  free(new_lines);

}
