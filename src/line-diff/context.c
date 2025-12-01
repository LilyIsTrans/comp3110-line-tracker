#include "context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define MAX_TOKENS 50
#define MAX_TOKEN_LEN 50
#define MAX_STRINGS 100

/*
typedef struct {
    char tokens[MAX_TOKENS][MAX_TOKEN_LEN];
    int count;
} TokenList;

typedef struct {
    char token[MAX_TOKEN_LEN];
    int count_a;
    int count_b;
} TokenCount;

typedef struct {
    char* candidate;
    double similarity;
} comparisonResult;
*/

TokenList extract_tokens(const char *text) {
    TokenList list = {.count = 0};
    int len = strlen(text);
    int start = -1;
    
    for (int i = 0; i <= len; i++) {
        char c = text[i];
        int is_token_char = (isalnum(c) || c == '_' || c == '-');
        
        if (is_token_char && start == -1) start = i;
        else if ((!is_token_char || i == len) && start != -1) {
            int token_len = i - start;
            if (token_len > 0 && token_len < MAX_TOKEN_LEN && list.count < MAX_TOKENS) {
                strncpy(list.tokens[list.count], text + start, token_len);
                list.tokens[list.count][token_len] = '\0';
                
                // Lowercase in-place
                for (int j = 0; list.tokens[list.count][j]; j++) {
                    list.tokens[list.count][j] = tolower(list.tokens[list.count][j]);
                }
                list.count++;
            }
            start = -1;
        }
    }
    return list;
}

double cosine_similarity(const char *text_a, const char *text_b) {
    TokenList tokens_a = extract_tokens(text_a);
    TokenList tokens_b = extract_tokens(text_b);
    
    if (tokens_a.count == 0 || tokens_b.count == 0) return 0.0;
    
    TokenCount token_counts[MAX_TOKENS * 2];
    int token_count = 0;
    
    for (int i = 0; i < tokens_a.count; i++) {
        int found = 0;
        for (int j = 0; j < token_count; j++) {
            if (strcmp(token_counts[j].token, tokens_a.tokens[i]) == 0) {
                token_counts[j].count_a++;
                found = 1;
                break;
            }
        }
        if (!found && token_count < MAX_TOKENS * 2) {
            strcpy(token_counts[token_count].token, tokens_a.tokens[i]);
            token_counts[token_count].count_a = 1;
            token_counts[token_count].count_b = 0;
            token_count++;
        }
    }
    
    for (int i = 0; i < tokens_b.count; i++) {
        int found = 0;
        for (int j = 0; j < token_count; j++) {
            if (strcmp(token_counts[j].token, tokens_b.tokens[i]) == 0) {
                token_counts[j].count_b++;
                found = 1;
                break;
            }
        }
        if (!found && token_count < MAX_TOKENS * 2) {
            strcpy(token_counts[token_count].token, tokens_b.tokens[i]);
            token_counts[token_count].count_a = 0;
            token_counts[token_count].count_b = 1;
            token_count++;
        }
    }
    
    double dot_product = 0.0, mag_a = 0.0, mag_b = 0.0;
    for (int i = 0; i < token_count; i++) {
        dot_product += token_counts[i].count_a * token_counts[i].count_b;
        mag_a += token_counts[i].count_a * token_counts[i].count_a;
        mag_b += token_counts[i].count_b * token_counts[i].count_b;
    }
    
    if (mag_a == 0.0 || mag_b == 0.0) return 0.0;
    return dot_product / (sqrt(mag_a) * sqrt(mag_b));
}

comparisonResult* compare_all_pairs(char* query, char** strings, int num_strings) {
    comparisonResult* results = (comparisonResult *) malloc(num_strings * sizeof(comparisonResult));
    for (int j = 0; j < num_strings; j++) {
        double similarity = cosine_similarity(query, strings[j]);
        results[j].candidate = strings[j];
        results[j].similarity = similarity;
    }
    return results;
}

/*
int main() {
    char *candidates[] = {
        "user-name = first_name + last_name",
        "user-name = firstname + lastname", 
        "user-name = firstname + last_name",
        "username = firstname + last_name",
        "user_profile = get_profile_data(user_id)",
        "user_profile = fetch_user_data(user_id)",
        "user_data = get_profile_info(user_id)",
        "user_info = retrieve_profile_data(user_id)",
        "customer_name = first_name + last_name",
        "client_profile = obtain_user_data(client_id)"
    };
    int num_candidates = sizeof(candidates) / sizeof(candidates[0]);
    
    char *query = "user-name = first_name + last_name";
    
    comparisonResult* results;
    results = compare_all_pairs(query, candidates, num_candidates);
    for (int i=0; i<num_candidates; i++) {
        printf("%s: %.3f\n", results[i].candidate, results[i].similarity);
    }
    return 0;
}
    */