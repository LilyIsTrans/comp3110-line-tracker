#pragma once
#include <stdio.h>

/**
  *  set the XML file to track
  *
  *	 xml_file:	file to track
  */
void xml_set_output_file(FILE *xml_file);

/**
  *  starts a new XML version block for the given file.
  *
  *  filename:	name of the file being tracked.
  *  id:		id for this version block.
  */
void start_version_block(const char* filename, int id);

/**
  *  outputs a line change in XML formatting.
  *
  *  old_line:	line number in the old version.
  *  new_line:	line number in the new version.
  */
void write_location(int old_line, int new_line);


/**
  *  ends the current XML version block.
  */
void end_version_block(void);
