#include "../string-slice/string-slice.h"
#include "../load-file/load.h"
#include "../line-diff/filter.h"
#include "../line-diff/distance.h"
#include "../line-diff/context.h"
#include "../split-lines/detect-split.h"
#include "main.h"
#include <stdio.h>

#define MIN_SIMILARITY 0.5

int main(int argc, const char **argv) {
    if (argc != 3) {
        printf("usage: <program> <filever1> <filever2>");
        return 1;
    }
    printf("line 17\n");
    // load files
    char* old_file_name = (char*)argv[1];
    char* new_file_name = (char*)argv[2];
    printf("line 21\n");
    struct LoadedFile* old_file = load_from_filename(old_file_name);
    struct LoadedFile* new_file = load_from_filename(new_file_name);
    printf("line 24\n");
    Substring old_file_content = {.start=old_file->data, .end=old_file->data + old_file->length};
    Substring new_file_content = {.start=new_file->data, .end=new_file->data + new_file->length};
    printf("line 27\n");
    // split into lines
    struct SubstringArray* old_file_lines = lines(old_file_content);
    struct SubstringArray* new_file_lines = lines(new_file_content);
    printf("line 31\n");
    // filter exact matches
    struct SubstringWithOriginLineArray unmatched = unmatched_lines(old_file_lines, new_file_lines);
    struct SubstringWithOriginLineArray allCandidates = unmatched_lines(new_file_lines, old_file_lines);
    printf("line 35\n");
    // iterate through unmatched lines from file 1
    struct SubstringWithOriginLineArray* filtered;
    Substring curString;
    size_t curLine;
    for (int i=0; i<unmatched.len; i++) {
        printf("line 41\n");
        // filter unmatched lines from file 2 by simhash
        curString = unmatched.strings[i];
        curLine = unmatched.lines[i];
        filtered = filterCandidates(&curString, &allCandidates);
        printf("line 46\n");
        // iterate over remaining 15 lines
        double maxSimilarity = -1;
        Substring maxCandidate;
        size_t maxCandidateLine;
        int maxSplitLines;
        for (int j=0; j<MAX_CANDIDATES; j++) {
            printf("line 53\n");
            Substring candidate = filtered->strings[j];
            size_t candidateLine = filtered->lines[j];
            Substring* candidatePrevContextSubstring = old_file_lines->array+(sizeof(Substring)*(candidateLine-3));
            Substring* candidateInContextSubstring = old_file_lines->array+(sizeof(Substring)*(candidateLine));
            printf("line 58\n");
            char* candidatePrevContext[3] = {(char*)candidatePrevContextSubstring[0].start, (char*)candidatePrevContextSubstring[1].start, (char*)candidatePrevContextSubstring[2].start};
            printf("line 60\n");
            bool split;
            // run levenshtein distance for one line
            double singleSimilarity =(double) levenshteinDistance(curString, candidate);
            singleSimilarity /= curString.end - curString.start;
            singleSimilarity = 1-singleSimilarity;
            printf("line 66\n");
            // calculates distance for split string
            double splitSimilarity;
            int best;
            checkSplit(curString, candidateInContextSubstring, old_file_lines->len - candidateLine, &best, &splitSimilarity);
            printf("line 71\n");
            // compares distances and chooses the closer option
            if (singleSimilarity>splitSimilarity) {
                best = 1;
                split = false;
            } else {
                split = true;
            }
            printf("line 79\n");
            Substring* candidatePostContextSubstring = old_file_lines->array+(sizeof(Substring)*(candidateLine+best));
            char* candidatePostContext[3] = {(char*)candidatePostContextSubstring[0].start, (char*)candidatePostContextSubstring[1].start, (char*)candidatePostContextSubstring[2].start};
            printf("line 82\n");
            // calculate cosine similarities of the line's context based on 3 lines above/below
                // note for split lines this means 3 lines after the last split line
            double contextSimilarity = cosine_similarity(combineStrings(candidatePrevContext, 3), combineStrings(candidatePostContext, 3));
            double similarity = 0.6 * ((split)? splitSimilarity : singleSimilarity) + 0.4 * contextSimilarity;
            printf("line 87\n");
            // based on combined values of levenshtein distance and cosine similarity, determine the closest candidate
            if (similarity > maxSimilarity) {
                printf("line 90\n");
                maxSimilarity = similarity;
                maxCandidate = candidate;
                maxCandidateLine = maxCandidateLine;
                maxSplitLines = best;
            }
        }
        // if the closest candidate is above some threshold, they are a match
        if (maxSimilarity > MIN_SIMILARITY) {
            printf("line 99\n");
            // match
            if (maxSplitLines > 1) {
                printf("Modified %s at line %d to %s at %d - %d\n", curString.start, (int)curLine, maxCandidate.start, (int)maxCandidateLine, (int)maxCandidateLine+maxSplitLines);
            } else {
                printf("Modified %s at line %d to %s at %d\n", curString.start, (int)curLine, maxCandidate.start, (int)maxCandidateLine);
            }
            // remove from list of candidates

        } else { // if not, the line is removed in version 2
            printf("line 108\n");
            // removal
            printf("Remove %s from line %d\n", curString.start, (int)curLine);
            // no effect on candidates
        }
    }
    printf("line 113");
    // any remaining lines in version 2 are insertions
    for (int k = 0; k<allCandidates.len; k++) {
        Substring c = allCandidates.strings[k];
        size_t l = allCandidates.lines[k];

        printf("Insert %s at line %d\n", c.start, (int)l);
    }

    return 0;
}