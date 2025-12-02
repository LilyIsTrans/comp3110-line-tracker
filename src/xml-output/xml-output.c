#include "xml-output.h"
#include <stdio.h>

FILE *xml_file;

void set_xml_output_file(FILE* file) {
    xml_file = file;
}

// start new xml '<VERSION>' block
void start_version_block(const char* filename, int id) {
    if (!xml_file) {
        return;
    }
    fprintf(xml_file, "<VERSION file=\"%s\" id=\"%d\">\n", filename, id);
}

 // write a '<LOCATION>' line inside the current version block
void write_location(int old_line, int new_line) {
    if (!xml_file) {
        return;
    }
    fprintf(xml_file, "<LOCATION old=\"%d\" new=\"%d\" />\n", old_line, new_line);
}

// close current '<VERSION>' block
void end_version_block(void) {
    if (!xml_file) {
        return;
    }
    fprintf(xml_file, "</VERSION>\n");
}
