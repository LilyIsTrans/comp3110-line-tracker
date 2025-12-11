#include "load.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if (defined(__unix) || defined(__unix__)) && !defined(FORCE_PORTABLE_FALLBACK)
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
#elif defined(__WIN64) || defined(__WIN64__)
#define WIN32_LEAN_AND_MEAN
#include <handleapi.h>
#include <memoryapi.h>
#include <processthreadsapi.h>
#include <windows.h>
#include <windef.h>
#include <memoryapi.h>
#include <fileapi.h>
#include <winnt.h>
struct LoadedFile {
  const char* data;
  size_t length: sizeof(size_t) * 8 - 1;
  // If true, file is `mmap`ed, and must be `munmap`ed to cleanup.
  // If false, file is `malloc`ed, and must be `free`d to cleanup.
  bool mapped: 1;
};

struct LoadedFile* load_from_filename(const char* const filename) {
  struct LoadedFile *output = malloc(sizeof(struct LoadedFile));
  HANDLE file = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (file == NULL) {
    fprintf(stderr, "Failed to open file '%s'!\n", filename);
    // Code from https://learn.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
    char *lpMsgBuf;
    DWORD dw = GetLastError(); 

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL) == 0) {
        fprintf(stderr, "Failed to even format the error!! Yikes!!\n");
        ExitProcess(dw);
    }

    fprintf(stderr, "Failed to open file BECAUSE: %s\n", lpMsgBuf);
    LocalFree(lpMsgBuf);
    ExitProcess(dw);
  }
  HANDLE mapping = CreateFileMapping2(file, NULL, FILE_MAP_READ, PAGE_READONLY, 0, 0, NULL, NULL, 0);
  if (mapping == NULL) {
    fprintf(stderr, "Failed to map file '%s'!\n", filename);
    char* lpMsgBuf;
    DWORD dw = GetLastError(); 

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL) == 0) {
        fprintf(stderr, "Failed to even format the error!! Yikes!!\n");
        ExitProcess(dw);
    }

    fprintf(stderr, "Failed to map file BECAUSE: %s\n", lpMsgBuf);
    LocalFree(lpMsgBuf);
    ExitProcess(dw);
  }

  output->data = MapViewOfFile(mapping, FILE_MAP_COPY, 0, 0, 0);
  if (output->data == NULL) {
    fprintf(stderr, "Failed to map view of file '%s'!\n", filename);
    char* lpMsgBuf;
    DWORD dw = GetLastError(); 

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL) == 0) {
        fprintf(stderr, "Failed to even format the error!! Yikes!!\n");
        ExitProcess(dw);
    }

    fprintf(stderr, "Failed to map view of file BECAUSE: %s\n", lpMsgBuf);
    LocalFree(lpMsgBuf);
    ExitProcess(dw);
    
  }
  output->mapped = true;
  LARGE_INTEGER len;
  if (!GetFileSizeEx(file, &len)) {
    fprintf(stderr, "Failed to map view of file '%s'!\n", filename);
    char *lpMsgBuf;
    DWORD dw = GetLastError(); 

    if (FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL) == 0) {
        fprintf(stderr, "Failed to even format the error!! Yikes!!\n");
        ExitProcess(dw);
    }

    fprintf(stderr, "Failed to map view of file BECAUSE: %s\n", lpMsgBuf);
    LocalFree(lpMsgBuf);
    ExitProcess(dw);    
    
  }
  output->length = (size_t)len.HighPart << 8 * sizeof(len.HighPart) | (size_t)len.LowPart;
  CloseHandle(file);
  CloseHandle(mapping);

  return output;
}

void unload_file(struct LoadedFile *file) {
  if (file->mapped) {
    UnmapViewOfFile(file->data);
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
  const char* data;    //hold contents of file
  size_t length;       //size of file (bytes)
};

struct LoadedFile* load_from_filename(const char* const filename) {
  FILE* file = fopen(filename, "rb");

  //ERROR HANDLING
  if (file == NULL) {
    fprintf(stderr, "Failed to open file '%s', reason: '%s'.\n", filename, strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (fseek(file, 0, SEEK_END)) {
    fprintf(stderr, "Failed to seek to end of file '%s'. Either your libc is broken or your computer is haunted. Officially, the error was '%s'. The stream error indicator was %d (0 indicates no error).\n", filename, strerror(errno), ferror(file));
    exit(EXIT_FAILURE);
  }
  int filesize_temp = ftell(file);
  if (filesize_temp == -1) {
    fprintf(stderr, "Failed to read cursor position for file '%s'. Either your libc is broken or your computer is haunted. Officially, the error was '%s'. The stream error indicator was %d (0 indicates no error).\n", filename, strerror(errno), ferror(file));
    exit(EXIT_FAILURE);
  }

  //file size in bytes (saved for later)
  size_t filesize = filesize_temp;

  //allocate memory to hold file contents
  char* data = malloc(filesize);

  //ERROR HANDLING
  if (data == NULL) {
    fprintf(stderr, "Failed to allocate %zu bytes to store the data of file '%s'. Reason: %s.\n", filesize, filename, strerror(errno));
    exit(EXIT_FAILURE);
  }

  //reset file cursor to beginning of file
  if (fseek(file, 0, SEEK_SET)) {
    fprintf(stderr, "Failed to seek to start of file '%s'. Either your libc is broken or your computer is haunted. Officially, the error was '%s'. The stream error indicator was %d (0 indicates no error).\n", filename, strerror(errno), ferror(file));
    exit(EXIT_FAILURE);
  }

  //total num of bytes read so far
  size_t bytes_read = 0;

  while (true) {
      //read from file into data buffer
      size_t this_read = fread(data + bytes_read, 1, filesize - bytes_read, file);
      fprintf(stderr, "Read %zu bytes from file '%s'\n.", this_read, filename);
      
      //ERROR HANDLING
      if (ferror(file)) {
        fprintf(stderr, "Failed to read from file '%s' at position %zu with %zu bytes to go because '%s'. The stream error indicator was %d (0 indicates no error).\n", filename, bytes_read, filesize - bytes_read, strerror(errno), ferror(file));
        exit(EXIT_FAILURE);
      }
      //check if entire file has been read
      else if (bytes_read + this_read == filesize) {
        break;
      }
      //ERROR HANDLING
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

  struct LoadedFile output_data = { data, filesize }; 		        //initialize struct
  struct LoadedFile *output = malloc(sizeof(struct LoadedFile));    //allocate memory for struct
  *output = output_data;                                            //copy data into allocated struct
  return output;
  
}

void unload_file(struct LoadedFile *file) {
  free((void*)file->data);   //free memory allocated for file data
  free(file);                //free memory allocated for struct
}

#endif

//return pointer to file data
const char* get_data_ptr(struct LoadedFile* file)  {
  return file->data;
}

//return size of file (bytes)
size_t get_size(struct LoadedFile* file)  {
  return file->length;
}
