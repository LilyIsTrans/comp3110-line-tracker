#include "distance.h"
#include "context.h"
#include "filter.h"
#include "../string-slice/string-slice.h" // note: have to link this when compiling based on my testing
#include <stdio.h>
#include <inttypes.h>

#define MAX_CANDIDATES 15

uint32_t simhash(Substring *str) {
    int32_t vector[32] = {0};
    
    uint32_t hash = 0;
    for (const char* s = str->start; s < str->end; s++) {
        hash = hash * 31 + (uint32_t)(*s);
    }

    hash ^= hash >> 16;
    hash *= 0x85ebca6bU;
    hash ^= hash >> 13;
    
    for (int i = 0; i < 32; i++) {
        vector[i] += (hash & (1U << i)) ? 1 : -1;
    }
    
    for (const char* s = str->start; s < str->end; s++) {
        uint32_t rolling = (uint32_t)(*s) * 0x85ebca6bU;
        for (int i = 0; i < 32; i++) {
            vector[i] += (rolling & (1U << i)) ? 1 : -1;
        }
    }
    
    uint32_t result = 0;
    for (int i = 31; i >= 0; i--) {
        result <<= 1;
        if (vector[i] > 0) result |= 1;
    }
    return result;
}

int hammingDistance(uint32_t a, uint32_t b) {
    uint32_t diff = a ^ b;
    int dist = 0;
    while (diff) {
        dist++;
        diff &= diff - 1;
    }
    return dist;
}

struct SubstringArray* filterCandidates(Substring *str, struct SubstringArray *candidates) {
    struct SubstringArray* filtered = new_substring_array(MAX_CANDIDATES);
    Substring *res = filtered->array;
    int resDist[MAX_CANDIDATES];
    
    // Initialize all to -1
    for (int i = 0; i < MAX_CANDIDATES; i++) {
        resDist[i] = -1;
    }
    
    int filled = 0;
    Substring candidate;
    
    for (int i = 0; i < candidates->len; i++) {
        candidate = candidates->array[i];
        int dist = hammingDistance(simhash(str), simhash(&candidate));
        
        // If we haven't filled the array yet, just add it
        if (filled < MAX_CANDIDATES) {
            // Find position to insert (keep sorted by distance)
            int pos = filled;  // Default to append at the end
            
            for (int j = 0; j < filled; j++) {
                if (dist < resDist[j]) {
                    // Shift elements to make room
                    for (int k = filled; k > j; k--) {
                        if (k < MAX_CANDIDATES) {
                            resDist[k] = resDist[k-1];
                            res[k] = res[k-1];
                        }
                    }
                    pos = j;
                    break;
                }
            }
            
            resDist[pos] = dist;
            res[pos] = candidate;
            filled++;
        } else {
            // Array is full, replace if this is better than the worst
            int worstIdx = 0;
            for (int j = 1; j < MAX_CANDIDATES; j++) {
                if (resDist[j] > resDist[worstIdx]) {
                    worstIdx = j;
                }
            }
            if (dist < resDist[worstIdx]) {
                resDist[worstIdx] = dist;
                res[worstIdx] = candidate;
            }
        }
    }
    
    filtered->len = filled;
    return filtered;
}
/*
int main(int argc, const char **argv) {
    Substring base;
    base.start = "x = 5 + 5";
    base.end = base.start + strlen(base.start);
    char* canstrings[] = {
        "x = 5 + 3",
        "y = 5 - 3",
        "z = x * y",
        "result = z / 2",
        "print('Result:', result)",
        "a = 10 + 3",
        "b = 10 - 3",
        "c = a * b",
        "output = c / 2",
        "print('Output:', output)",
        "count = 0",
        "count = count + 1",
        "index = 0",
        "index = index + 1",
        "list_a = [1, 2, 3]",
        "list_b = [4, 5, 6]",
        "combined = list_a + list_b",
        "first = list_a[0]",
        "last = list_b[-1]",
        "if x > y:",
        " print('x is larger')",
        "elif a > b:",
        " print('a is larger')",
        "else:",
        " print('none larger')"
    };
    struct SubstringArray* candidates = new_substring_array(25);
    Substring canlist[25];
    for (int i=0; i<25; i++) {
        canlist[i].start = canstrings[i];
        canlist[i].end = canlist[i].start+strlen(canlist[i].start);
    }
    candidates->len = 25;
    memcpy(candidates->array, canlist, sizeof(Substring)*25);
    struct SubstringArray* filtered = filterCandidates(&base, candidates);
    for (int i = 0; i<filtered->len; i++) {
        printf("%s,\n %s: %d\n", base.start, filtered->array[i].start, hammingDistance(simhash(&base),simhash(&filtered->array[i])));
        
    }
    return 0;
}
    */