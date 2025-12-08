#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define MAX_TOKENS 50
#define MAX_TOKEN_LEN 50
#define MAX_STRINGS 100

typedef struct {
    char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
    int count;
} TokenList;

typedef struct {
    char token[MAX_TOKEN_LEN];
    int count_a;
    int count_b;
} TokenCount;

/*
    temporary token extraction to grab a list, use the tokens for cosine similarity
    takes the whole text string as input and outputs a list of tokens
*/
TokenList extract_tokens(const char *text);

/*
    calculates cosine similarity between two strings
    takes two strings as input, converts them to vectors representing shared tokens, returns the cosine similarity based on these vectors
*/
double cosine_similarity(const char *text_a, const char *text_b);

typedef struct {
    char* candidate;
    double similarity;
} comparisonResult;

/*
    returns an array of the cosine similarity results
    takes in a query and a list of candidates (and the length of this list)
*/
comparisonResult* compare_all_pairs(char* query, char** strings, int num_strings);