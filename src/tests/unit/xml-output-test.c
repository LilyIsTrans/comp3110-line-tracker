#include "../../xml-output/xml-output.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {

    //temp file for test purposes
    FILE* test_file = fopen("xml_output_test.xml", "a");
    if (!test_file) {
        perror("Failed to open test file");
        return EXIT_FAILURE;
    }

    //set output file

    //start block
    start_version_block(test_file, "sample_file_name.c", 1);

    //example location entries:
    write_location(test_file, 10, 11);
    write_location(test_file, 25, 30);

    //end block
    end_version_block(test_file);

    fclose(test_file);

    return EXIT_SUCCESS;
}
