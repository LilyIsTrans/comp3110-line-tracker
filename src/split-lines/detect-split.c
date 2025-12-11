#include "../line-diff/distance.h"
#include "../string-slice/string-slice.h"
#include "detect-split.h"
#include <stdio.h>

char* combineStrings(char** strs, int len) {
    if (!strs || len <= 0) {
        char* empty = malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    
    // Calculate total length
    int combinedLen = 0;
    for (int i = 0; i < len; i++) {
        if (strs[i]) {
            combinedLen += strlen(strs[i]);
        }
    }
    
    // Allocate memory (+1 for null terminator)
    char* combined = malloc(combinedLen + 1);
    if (!combined) return NULL;
    
    // Copy strings
    int pos = 0;
    for (int i = 0; i < len; i++) {
        if (strs[i]) {
            int strLen = strlen(strs[i]);
            // Copy character by character (or use memcpy)
            for (int j = 0; j < strLen; j++) {
                combined[pos] = strs[i][j];
                pos++;
            }
        }
    }
    
    // Null terminate
    combined[pos] = '\0';
    return combined;
}

char* combineSubstrings(Substring* strs, int len) {
    if (len <= 0) return NULL;
    
    // Calculate total length
    int totalLen = 0;
    for (int i = 0; i < len; i++) {
        totalLen += (strs[i].end - strs[i].start);
    }
    
    // Allocate and copy
    char* combined = malloc(totalLen + 1);
    if (!combined) return NULL;
    
    int pos = 0;
    for (int i = 0; i < len; i++) {
        int subLen = strs[i].end - strs[i].start;
        if (subLen > 0) {
            memcpy(combined + pos, strs[i].start, subLen);
            pos += subLen;
        }
    }
    combined[pos] = '\0';
    return combined;
}
void checkSplit(Substring str, Substring strs[], int strscount, int* best, double* bestDiff) {
    double cur = 0;
    double prev;
    int combined = 2;
    
    char* merged = NULL;
    
    do {
        prev = cur;
        
        // Free previous merged string if it exists
        if (merged) {
            free(merged);
            merged = NULL;
        }
        
        // Combine substrings
        merged = combineSubstrings(strs, combined);
        if (!merged) {
            // Handle allocation failure
            *best = 1;
            *bestDiff = 0.0;
            return;
        }
        
        // Create substring from merged string
        Substring mergedSubstring = {.start = merged, .end = merged + strlen(merged)};
        
        // Calculate similarity
        cur = (double)levenshteinDistance(str, mergedSubstring);
        size_t strLen = str.end - str.start;
        size_t mergedLen = strlen(merged);
        size_t maxLen = (strLen > mergedLen) ? strLen : mergedLen;
        
        if (maxLen > 0) {
            cur /= maxLen;
        }
        cur = 1 - cur; // Convert to similarity
        
        combined++;
        
    } while (cur > prev && combined < strscount);
    
    // Free the last allocated merged string
    if (merged) {
        free(merged);
    }
    
    // Go one back (since we incremented combined before checking)
    *bestDiff = prev;
    *best = combined - 2;  // -2 because: we started at 2, and we want the previous value
    
    // Make sure best is at least 1
    if (*best < 1) {
        *best = 1;
    }
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