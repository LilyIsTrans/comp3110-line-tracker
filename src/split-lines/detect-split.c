#include "../line-diff/distance.h"
#include "../string-slice/string-slice.h"
#include "detect-split.h"
#include <stdio.h>

char* combineStrings(char** strs, int len) {
    int combinedLen=0;
    for (int i=0; i<len; i++) {
        combinedLen+=strlen(strs[i]);
    }
    char* combined;
    combined = malloc(combinedLen);
    int filled = 0;
    for (int i=0; i<len; i++) {
        for (int j=0; j < strlen(strs[i]); j++) {
            combined[filled+j] = strs[i][j];
        }
        //memcpy(combined[filled], strs[i], strlen(strs[i]));
        filled+=strlen(strs[i]);
        if (combined[filled] == '\0' || combined[filled] == '\n') {
            filled--;
        }
    }
    combined[filled+1]='\0';
    return combined;
}

char* combineSubstrings(Substring* strs, int len) {
    char* combined = (char*)strs[0].start;
    for (int i=1; i<len; i++) {
        char* toCombine[] = {combined, (char*)strs[i].start}; 
        combined = combineStrings(toCombine, 2);
    }
    return combined;
}
void checkSplit(Substring str, Substring strs[], int strscount, int* best, double* bestDiff) {
    double cur = 0;
    double prev;
    int combined=2;
    do {
        prev = cur;
        char* merged = combineSubstrings(strs, combined);
        Substring mergedSubstring = {.start=merged, .end=merged+strlen(merged)};
        cur = (double)levenshteinDistance(str, mergedSubstring);
        cur /= max(str.end-str.start, strlen(merged)); // % different between lines
        cur = 1-cur; // % similarity between lines
        combined++;
    } while (cur > prev && combined < strscount);

    // go one back
    *bestDiff = prev;
    *best = combined-2;
}
/*
int main(int argc, const char **argv) {
    char* str = "for (const char* p = tokens->array[i].start; p < tokens->array[i].end; p++) {";
    char* strs[] = {"for (\n",
                    "const char* p =\n",
                    " tokens->array[i].start;\n",
                    " p < tokens->array[i].end;\n",
                    " p++) {\n",
                    "test\n",
                    "testx2\n"};

    char* badstrs[] = {"void checkSplit(\n",
                    "char* str,\n",
                    " char* strs[],\n",
                    " int strscount,\n",
                    " int* best,\n",
                    " double* bestDiff\n",
                    ") {"};

    int best;
    double diff;
    checkSplit(str, strs, sizeof(strs)/sizeof(strs[0]), &best, &diff);
    printf("combined %d strings with %.3f similarity.\nCompared %s and %s\n", best, diff, str, combineStrings(strs, best));
    checkSplit(str, badstrs, sizeof(badstrs)/sizeof(badstrs[0]), &best, &diff);
    printf("combined %d strings with %.3f similarity.\nCompared %s and %s\n", best, diff, str, combineStrings(badstrs, best));
    return 0;
}
    */