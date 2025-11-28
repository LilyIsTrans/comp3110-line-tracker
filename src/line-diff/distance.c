#include "distance.h"
#include <stdio.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

int levenshteinDistance(const char* str1,
                        const char* str2, 
                        const unsigned int len1,
                        const unsigned int len2) {

    if (len2 < len1) {
        return levenshteinDistance(str2, str1, len2, len1);
    }

    int * prevRow;
    int * currRow;
    currRow = (int *) malloc((len1 + 1) * sizeof(int));
    prevRow = (int *) malloc((len1 + 1) * sizeof(int));

    for (int j = 0; j <= len1; j++) {
        prevRow[j] = j;
    }

    for (int i = 1; i <= len2; i++) {
        currRow[0] = i;

        for (int j = 1; j <= len1; j++) {
            if (str1[j - 1] == str2[i - 1]) {
                currRow[j] = prevRow[j - 1];
            }
            else {
                currRow[j] = 1 + min(currRow[j - 1], min(prevRow[j], prevRow[j - 1]));
            }
        }
        int * tmp = currRow;
        currRow = prevRow;
        prevRow = tmp;
    }

    int res = prevRow[len1];
    free(currRow);
    free(prevRow);
    return res;
}

void closestLine(char* str1, char* strs[], int strscount, char** best, int* bestDiff) {
    *bestDiff = -1;
    for(int i=0; i<strscount; i++) {
        int diff = levenshteinDistance(str1, strs[i], strlen(str1), strlen(strs[i]));
        if (*bestDiff ==-1) {
            *bestDiff = diff;
            *best = strs[i];
        }
        if (diff < *bestDiff) {
            *bestDiff = diff;
            *best = strs[i];
        }
    }
}

int main(int argc, const char **argv) {
    char* str1 = "abcdef";
    char* strs[] =  {
        "abcde",
        "bcd",
        "test",
        "tictac",
    };
    int d;
    char* best;

    closestLine(str1, strs, sizeof(strs)/sizeof(strs[0]), &best, &d);

    printf("%s, %s: %d", str1, best, d);
    
        return 0;
}
