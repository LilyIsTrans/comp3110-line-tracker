#pragma once
#include <stdio.h>

/**
 *  starts a new XML version block for the given file.
 *
 * filename				name of the file being tracked.
 * id					id for this version block.
 */
void start_version_block(const char* filename, int id);

/**
 *  outputs a (single) line change in XML formatting.
 *
 *  old_line			line number in the old version.
 *  new_line			line number in the new version.
 */
void write_location(int old_line, int new_line);


//  ends the current XML version block.
void end_version_block(void);
