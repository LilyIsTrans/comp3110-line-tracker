#include "../string-slice/string-slice.h"
#include "../load-file/load.h"
#include "../line-diff/filter.h"
#include "../line-diff/distance.h"
#include "../line-diff/context.h"
#include "../split-lines/detect-split.h"
#include "main.h"
#include <stdio.h>

#define MIN_SIMILARITY 0.5

char* substring_to_cstring(Substring s) {
    size_t len = s.end - s.start;
    char* result = malloc(len + 1);
    if (!result) return NULL;
    memcpy(result, s.start, len);
    result[len] = '\0';
    return result;
}

int main(int argc, const char **argv) {
    if (argc != 3) {
        printf("usage: <program> <filever1> <filever2>");
        return 1;
    }
    // load files
    char* old_file_name = (char*)argv[1];
    char* new_file_name = (char*)argv[2];
    struct LoadedFile* old_file = load_from_filename(old_file_name);
    struct LoadedFile* new_file = load_from_filename(new_file_name);
    Substring old_file_content = {.start=old_file->data, .end=old_file->data + old_file->length};
    Substring new_file_content = {.start=new_file->data, .end=new_file->data + new_file->length};
    // split into lines
    struct SubstringArray* old_file_lines = lines(old_file_content);
    struct SubstringArray* new_file_lines = lines(new_file_content);
    // filter exact matches
    struct SubstringWithOriginLineArray unmatched = unmatched_lines(old_file_lines, new_file_lines);
    struct SubstringWithOriginLineArray allCandidates = unmatched_lines(new_file_lines, old_file_lines);
    // iterate through unmatched lines from file 1
    struct SubstringWithOriginLineArray* filtered;
    Substring curString;
    size_t curLine;

    for (int i=0; i<unmatched.len; i++) {
        // filter unmatched lines from file 2 by simhash
        curString = unmatched.strings[i];
        curLine = unmatched.lines[i];
        filtered = filterCandidates(&curString, &allCandidates);
        if (!filtered) {
            continue;  // Skip to next unmatched line
        }
        if (filtered->len == 0) {
            continue;
        }
        // iterate over remaining 15 lines
        double maxSimilarity = -1;
        Substring maxCandidate;
        size_t maxCandidateLine;
        int maxSplitLines;

        bool prevContextOrig = true;
        bool postContextOrig = true;
        if (curLine >= old_file_lines->len) {
            continue;
        }
        if (curLine < 3) {
            prevContextOrig = false;
        }

        char* origPrevContext[3];
        if (prevContextOrig) {
            Substring* origPrevContextSubstring = &old_file_lines->array[curLine-3];
            bool prevContextOrigOK = true;
            for (int k = 0; k<3; k++) {
                origPrevContext[k] = substring_to_cstring(origPrevContextSubstring[k]);
                if (!origPrevContext[k]) {
                    prevContextOrigOK = false;
                    for (int m=0; m<k; m++) free(origPrevContext[m]);
                    break;
                }
            }
            if (!prevContextOrigOK) {
                prevContextOrig = false;
            }
        }

        int candidateCount = min(MAX_CANDIDATES, filtered->len);
        for (int j=0; j<candidateCount; j++) {
            bool prevContext = true;
            bool postContext = true;
            if (!filtered->strings) {
                break;
            }
            Substring candidate = filtered->strings[j];
            size_t candidateLine = filtered->lines[j];
            if (candidateLine >= new_file_lines->len) {
                continue;
            }
            if (candidateLine < 3) {
                prevContext = false;
            }
            Substring* candidateInContextSubstring = &new_file_lines->array[candidateLine];
            char* candidatePrevContext[3];
            if (prevContext) {
                Substring* candidatePrevContextSubstring = &new_file_lines->array[candidateLine-3];
                bool prevContextOK = true;
                for (int k = 0; k<3; k++) {
                    candidatePrevContext[k] = substring_to_cstring(candidatePrevContextSubstring[k]);
                    if (!candidatePrevContext[k]) {
                        prevContextOK = false;
                        for (int m=0; m<k; m++) free(candidatePrevContext[m]);
                        break;
                    }
                }
                if (!prevContextOK) {
                    prevContext = false;
                }
            }
            bool split;
            // run levenshtein distance for one line
            double singleSimilarity =(double) levenshteinDistance(curString, candidate);
            singleSimilarity /= curString.end - curString.start;
            singleSimilarity = 1-singleSimilarity;
            // calculates distance for split string
            double splitSimilarity;
            int best = 0;
            checkSplit(curString, candidateInContextSubstring, new_file_lines->len - candidateLine, &best, &splitSimilarity);
            // compares distances and chooses the closer option
            if (singleSimilarity>splitSimilarity) {
                best = 1;
                split = false;
            } else {
                split = true;
            }
            if (candidateLine + best + 2 >= new_file_lines->len) {  // Need 3 lines after
                postContext = false;
            }
            char* candidatePostContext[3]; 
            if (postContext) {
                Substring* candidatePostContextSubstring = &new_file_lines->array[candidateLine+best];
                bool postContextOK = true;
                for (int k = 0; k<3; k++) {
                    candidatePostContext[k] = substring_to_cstring(candidatePostContextSubstring[k]);
                    if (!candidatePostContext[k]) {
                        postContextOK = false;
                        for (int m=0; m<3; m++) free(candidatePrevContext[m]);
                        for (int m=0; m<3; m++) free(candidatePostContext[m]);
                        break;
                    }
                }
                if (!postContextOK) {
                    continue;
                }
            }
    
            if (curLine + best + 2 >= old_file_lines->len) {  // Need 3 lines after
                postContextOrig = false;
            }
            char* origPostContext[3]; 
            if (postContextOrig) {
                Substring* origPostContextSubstring = &old_file_lines->array[curLine+best];
                bool postContextOrigOK = true;
                for (int k = 0; k<3; k++) {
                    origPostContext[k] = substring_to_cstring(origPostContextSubstring[k]);
                    if (!origPostContext[k]) {
                        postContextOrigOK = false;
                        for (int m=0; m<3; m++) free(origPrevContext[m]);
                        for (int m=0; m<3; m++) free(origPostContext[m]);
                        break;
                    }
                }
                if (!postContextOrigOK) {
                    postContextOrig = false;
                }
            }
            // calculate cosine similarities of the line's context based on 3 lines above/below
                // note for split lines this means 3 lines after the last split line
            char* prevCombined = NULL;
            char* postCombined = NULL;
            if (prevContext) {
                prevCombined = combineStrings(candidatePrevContext, 3);
                if (!prevCombined) {
                    for (int k=0; k<3; k++) free(candidatePrevContext[k]);
                    continue;
                }
            }
            if (postContext) {
                postCombined = combineStrings(candidatePostContext, 3);
                if (!postCombined) {
                    if (prevCombined) free(prevCombined);
                    for (int k=0; k<3; k++) {
                        free(candidatePrevContext[k]);
                        free(candidatePostContext[k]);
                    }
                    continue;
                }
            }

            char* prevCombinedOrig = NULL;
            char* postCombinedOrig = NULL;
            if (prevContextOrig) {  // Check orig context flag, not candidate flag
                prevCombinedOrig = combineStrings(origPrevContext, 3);
                if (!prevCombinedOrig) {
                    if (prevCombined) free(prevCombined);
                    if (postCombined) free(postCombined);
                    for (int k=0; k<3; k++) {
                        free(candidatePrevContext[k]);
                        free(candidatePostContext[k]);
                    }
                    continue;
                }
            }
            if (postContextOrig) {  // Check orig context flag, not candidate flag
                postCombinedOrig = combineStrings(origPostContext, 3);
                if (!postCombinedOrig) {
                    if (prevCombined) free(prevCombined);
                    if (postCombined) free(postCombined);
                    if (prevCombinedOrig) free(prevCombinedOrig);
                    for (int k=0; k<3; k++) {
                        free(candidatePrevContext[k]);
                        free(candidatePostContext[k]);
                        free(origPrevContext[k]);
                    }
                    continue;
                }
            }

            double prevContextSimilarity;
            double postContextSimilarity;
            if (prevContext && prevContextOrig) prevContextSimilarity = cosine_similarity(prevCombined, prevCombinedOrig);
            if (postContext && postContextOrig) postContextSimilarity = cosine_similarity(postCombined, postCombinedOrig);
            int divisor = 2;
            double similarity;
            double contextSimilarity; 
            if (!prevContext && !postContext) {
                similarity = ((split)? splitSimilarity : singleSimilarity);
            } else if (!prevContext) {
                contextSimilarity = 0.4 * postContextSimilarity;
                similarity = 0.6 * ((split)? splitSimilarity : singleSimilarity) + contextSimilarity;

            } else if (!postContext) {
                contextSimilarity = 0.4 * prevContextSimilarity;
                similarity = 0.6 * ((split)? splitSimilarity : singleSimilarity) + contextSimilarity;

            } else {
                contextSimilarity = 0.2 * prevContextSimilarity + 0.2 * postContextSimilarity;
                similarity = 0.6 * ((split)? splitSimilarity : singleSimilarity) + contextSimilarity;
            }
            // based on combined values of levenshtein distance and cosine similarity, determine the closest candidate
            if (similarity > maxSimilarity) {
                maxSimilarity = similarity;
                maxCandidate = candidate;
                maxCandidateLine = candidateLine;
                maxSplitLines = best;
                printf("Splits: %d\n", best);
            }
        }
        // if the closest candidate is above some threshold, they are a match
        if (maxSimilarity > MIN_SIMILARITY) {
            // match
            if (maxSplitLines > 1) {
                printf("Modified %s at line %d to %s at %d - %d\n", substring_to_cstring(curString), (int)curLine, substring_to_cstring(maxCandidate), (int)maxCandidateLine, (int)maxCandidateLine+maxSplitLines);
            } else {
                printf("Modified %s at line %d to %s at %d\n", substring_to_cstring(curString), (int)curLine, substring_to_cstring(maxCandidate), (int)maxCandidateLine);
            }
            // remove from list of candidates

        } else { // if not, the line is removed in version 2
            // removal
            printf("Remove %s from line %d\n", substring_to_cstring(curString), (int)curLine);
            // no effect on candidates
        }
    }
    // any remaining lines in version 2 are insertions
    for (int k = 0; k<allCandidates.len; k++) {
        Substring c = allCandidates.strings[k];
        size_t l = allCandidates.lines[k];

        printf("Insert %s at line %d\n", substring_to_cstring(c), (int)l);
    }

    return 0;
}