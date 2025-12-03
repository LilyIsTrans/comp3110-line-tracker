#include "../../xml-output/xml-output.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {

    //temp file for test purposes
    FILE* test_file = fopen("xml_output_test.xml", "w");
    if (!test_file) {
        perror("Failed to open test file");
        return EXIT_FAILURE;
    }

    //set output file
    set_xml_output_file(test_file);

    //start block
    start_version_block("sample_file_name.c", 1);

    //example location entries:
    write_location(10, 11);
    write_location(25, 30);

    //end block
    end_version_block();

    fclose(test_file);

    return EXIT_SUCCESS;
}
