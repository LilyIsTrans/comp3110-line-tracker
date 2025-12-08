#pragma once
#include "../line-diff/distance.h"
#include <stdio.h>

char* combineStrings(char** strs, int len);

void checkSplit(char* str, char* strs[], int strscount, int* best, double* bestDiff);