#include "xml-output.h"
#include <stdio.h>

// start new xml '<VERSION>' block
void start_version_block(FILE *xml_file, const char* filename, int id) {
    if (!xml_file) return;
    fprintf(xml_file, "<VERSION file=\"%s\" id=\"%d\">\n", filename, id);
}

 // write a '<LOCATION>' line inside the current version block
void write_location(FILE* xml_file, int old_line, int new_line) {
    if (!xml_file) return;
    fprintf(xml_file, "<LOCATION old=\"%d\" new=\"%d\" />\n", old_line, new_line);
}

// close current '<VERSION>' block
void end_version_block(FILE* xml_file) {
    if (!xml_file) return;
    fprintf(xml_file, "</VERSION>\n");
}
