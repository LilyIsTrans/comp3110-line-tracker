#pragma once
#include "../line-diff/distance.h"
#include "../string-slice/string-slice.h"
#include <stdio.h>

char* combineStrings(char** strs, int len);

char* combineSubstrings(Substring* strs, int len);

void checkSplit(Substring str, Substring strs[], int strscount, int* best, double* bestDiff);