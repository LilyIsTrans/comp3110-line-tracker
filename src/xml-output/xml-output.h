#pragma once
#include <stdio.h>

/**
  *  starts a new XML version block for the given file.
  *
  *	 xml_file:	file being tracked.
  *  filename:	name of the file being tracked.
  *  id:		id for this version block.
  */
void start_version_block(FILE *xml_file, const char* filename, int id);

/**
  *  outputs a line change in XML formatting.
  *
  *	 xml_file:	file to track
  *  old_line:	line number in the old version.
  *  new_line:	line number in the new version.
  */
void write_location(FILE *xml_file, int old_line, int new_line);


/**
  *  ends the current XML version block.
  * 
  *  xml_file:	file to track
  */
void end_version_block(FILE *xml_file);
