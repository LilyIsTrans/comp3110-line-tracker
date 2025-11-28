#pragma once
#include <stdio.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

/*
    Calculates the amount of letters added, removed, or modified between two strings
    takes two char arrays and their lengths, returns the distance between them
    O(m*n) efficiency where m is length of string 1 and n is length of string 2
*/
int levenshteinDistance(const char* str1,
                            const char* str2, 
                            const unsigned int len1,
                            const unsigned int len2);

/*
    Basic function to test proof of concept for a list of candidates comparing to one line
    takes in the string, a list of strings to compare to, the length of the list, and a pointer to a char array and int to store the best candidate and its score
*/
void closestLine(char* str1, char* strs[], int strscount, char** best, int* bestDiff);

// using main in this program to test closestLine
int main(int argc, const char **argv);