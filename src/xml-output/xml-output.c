#include "xml-output.h"
#include <stdio.h>


// start new xml '<VERSION>' block
void start_version_block(FILE* file, const char* filename, int id) {
    if (!file) {
        return;
    }
    fprintf(file, "<VERSION file=\"%s\" id=\"%d\">\n", filename, id);
}

 // write a '<LOCATION>' line inside the current version block
void write_location(FILE* file, int old_line, int new_line) {
    if (!file) {
        return;
    }
    fprintf(file, "<LOCATION old=\"%d\" new=\"%d\" />\n", old_line, new_line);
}

// close current '<VERSION>' block
void end_version_block(FILE* file) {
    if (!file) {
        return;
    }
    fprintf(file, "</VERSION>\n");
}
