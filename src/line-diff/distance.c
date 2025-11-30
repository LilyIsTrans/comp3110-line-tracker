#include "distance.h"
#include "context.h"
#include <stdio.h>

#define min(a, b) ((a) < (b) ? (a) : (b))


int levenshteinDistance(const char* str1,
                        const char* str2) {

    int len1 = strlen(str1);
    int len2 = strlen(str2);

    if (len2 < len1) {
        return levenshteinDistance(str2, str1);
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

void closestLine(Line str1, Line strs[], int strscount, Line* best, double* bestDiff) {
    *bestDiff = -1;
    for(int i=0; i<strscount; i++) {
        double dist = (double) levenshteinDistance(str1.line, strs[i].line);
        dist /= strlen(str1.line);
        dist = 1-dist;
        double context = (cosine_similarity(str1.prevContext, strs[i].prevContext) + cosine_similarity(str1.postContext, strs[i].postContext)) / 2;
        
        double diff = 0.6 * (double)dist + 0.4 * context;
        
        if (diff > *bestDiff) {
            *bestDiff = diff;
            *best = strs[i];
        }
    }
}

int main(int argc, const char **argv) {
    Line str1 = {"a b c", "abcdef", "d e f"};
    Line strs[] =  {
        {"a b c","abcde","d e f"},
        {"a x c","bcd","d x f"},
        {"a x x","test","d x x"},
        {"x x x","tictac","x x x"},
    };
    double d;
    Line best;

    closestLine(str1, strs, sizeof(strs)/sizeof(strs[0]), &best, &d);

    printf("%s, %s: %.3f\n", str1.line, best.line, d);
    
    return 0;
}
