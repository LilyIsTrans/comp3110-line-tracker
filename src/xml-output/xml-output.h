#pragma once
#include <stdio.h>

#if _MSC_VER
typedef ptrdiff_t ssize_t;
#else
#include <unistd.h>
#endif

/**
  *  starts a new XML version block for the given file.
  *
  *  filename:	name of the file being tracked.
  *  id:		id for this version block.
  */
void start_version_block(FILE* file, const char* filename, int id);

/**
  *  outputs a line change in XML formatting.
  *
  *  old_line:	line number in the old version.
  *  new_line:	line number in the new version.
  */
void write_location(FILE* file, ssize_t old_line, ssize_t new_line);


//ends the current XML version block.
void end_version_block(FILE* file);
