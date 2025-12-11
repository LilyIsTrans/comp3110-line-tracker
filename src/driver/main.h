#pragma once
#include "../string-slice/string-slice.h"
#include "../load-file/load.h"
#include "../line-diff/filter.h"
#include "../line-diff/distance.h"
#include "../line-diff/context.h"
#include "../split-lines/detect-split.h"
#include <stdio.h>

#define MIN_SIMILARITY 0.5

char* substring_to_cstring(Substring s);

int main(int argc, const char **argv);