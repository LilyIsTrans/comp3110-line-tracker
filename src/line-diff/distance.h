#pragma once
#include "context.h"
#include "../string-slice/string-slice.h"
#include <stdio.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

typedef struct{
    char* prevContext;
    char* line;
    char* postContext;
} Line;

/*
    Calculates the amount of letters added, removed, or modified between two strings
    takes two char arrays, returns the distance between them
    O(m*n) efficiency where m is length of string 1 and n is length of string 2
*/
int levenshteinDistance(Substring str1,
                        Substring str2);

/*
    Basic function to test proof of concept for a list of candidates comparing to one line
    takes in the string, a list of strings to compare to, the length of the list, and a pointer to a char array and int to store the best candidate and its score
*/
//void closestLine(Line str1, Line strs[], int strscount, Line* best, double* bestDiff);