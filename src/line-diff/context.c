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

// Define a proper struct for results
typedef struct {
    int index;
    double similarity;
    const char *text;
} SimilarityResult;

// Efficient token extraction
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

// Most efficient for short strings: array-based direct comparison
double array_direct_comparison(const char *text_a, const char *text_b) {
    TokenList tokens_a = extract_tokens(text_a);
    TokenList tokens_b = extract_tokens(text_b);
    
    if (tokens_a.count == 0 || tokens_b.count == 0) return 0.0;
    
    // Use fixed-size array (much faster for small token counts)
    TokenCount token_counts[MAX_TOKENS * 2];
    int token_count = 0;
    
    // Count tokens from first string
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
    
    // Count tokens from second string
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
    
    // Calculate cosine similarity
    double dot_product = 0.0, mag_a = 0.0, mag_b = 0.0;
    for (int i = 0; i < token_count; i++) {
        dot_product += token_counts[i].count_a * token_counts[i].count_b;
        mag_a += token_counts[i].count_a * token_counts[i].count_a;
        mag_b += token_counts[i].count_b * token_counts[i].count_b;
    }
    
    if (mag_a == 0.0 || mag_b == 0.0) return 0.0;
    return dot_product / (sqrt(mag_a) * sqrt(mag_b));
}

// Batch comparison - find most similar strings to a query
void find_similar_strings(const char *query, char **candidates, int num_candidates, int top_k) {
    printf("Query: \"%s\"\n", query);
    printf("Top %d most similar candidates:\n", top_k);
    
    // Store similarities
    SimilarityResult results[MAX_STRINGS];
    
    // Calculate all similarities
    for (int i = 0; i < num_candidates; i++) {
        results[i].index = i;
        results[i].similarity = array_direct_comparison(query, candidates[i]);
        results[i].text = candidates[i];
    }
    
    // Sort by similarity (simple bubble sort - fine for small lists)
    for (int i = 0; i < num_candidates - 1; i++) {
        for (int j = 0; j < num_candidates - i - 1; j++) {
            if (results[j].similarity < results[j + 1].similarity) {
                SimilarityResult temp = results[j];
                results[j] = results[j + 1];
                results[j + 1] = temp;
            }
        }
    }
    
    // Display top results
    for (int i = 0; i < top_k && i < num_candidates; i++) {
        printf("%d. [%.4f] %s\n", i + 1, results[i].similarity, results[i].text);
    }
    printf("\n");
}

// Compare all pairs efficiently
void compare_all_pairs(char **strings, int num_strings) {
    printf("=== All Pairwise Comparisons ===\n");
    printf("     ");
    for (int i = 0; i < num_strings; i++) printf("S%-2d    ", i + 1);
    printf("\n");
    
    for (int i = 0; i < num_strings; i++) {
        printf("S%-2d  ", i + 1);
        for (int j = 0; j < num_strings; j++) {
            double similarity = array_direct_comparison(strings[i], strings[j]);
            printf("%.3f  ", similarity);
        }
        printf("\n");
    }
}

// Simple version without sorting - just find top matches
void find_similar_strings_simple(const char *query, char **candidates, int num_candidates, int top_k) {
    printf("Query: \"%s\"\n", query);
    printf("Top %d most similar candidates:\n", top_k);
    
    double similarities[MAX_STRINGS];
    
    // Calculate all similarities
    for (int i = 0; i < num_candidates; i++) {
        similarities[i] = array_direct_comparison(query, candidates[i]);
    }
    
    // Find top k without full sorting
    for (int k = 0; k < top_k && k < num_candidates; k++) {
        int best_index = -1;
        double best_similarity = -1.0;
        
        for (int i = 0; i < num_candidates; i++) {
            // Skip already selected candidates
            int already_selected = 0;
            for (int j = 0; j < k; j++) {
                // Simple check: if this candidate was in top results before
                if (similarities[i] == -2.0) { // -2.0 means already selected
                    already_selected = 1;
                    break;
                }
            }
            
            if (!already_selected && similarities[i] > best_similarity) {
                best_similarity = similarities[i];
                best_index = i;
            }
        }
        
        if (best_index != -1) {
            printf("%d. [%.4f] %s\n", k + 1, best_similarity, candidates[best_index]);
            similarities[best_index] = -2.0; // Mark as selected
        }
    }
    printf("\n");
}

int main() {
    // Your candidate strings
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
    
    // Example queries
    char *queries[] = {
        "user-name = first_name + last_name",
        "user_profile = get_user_data(user_id)",
        "customer_profile = first_name + last_name"
    };
    int num_queries = sizeof(queries) / sizeof(queries[0]);
    
    printf("=== Finding Similar Strings ===\n\n");
    
    // Test each query with simple version (no struct issues)
    for (int i = 0; i < num_queries; i++) {
        find_similar_strings_simple(queries[i], candidates, num_candidates, 3);
    }
    
    // Show all pairwise comparisons
    compare_all_pairs(candidates, num_candidates);
    
    // Test individual comparisons
    printf("\n=== Key Comparisons ===\n");
    printf("'user-name = first_name + last_name' vs 'user-name = firstname + lastname': %.4f\n",
           array_direct_comparison(candidates[0], candidates[1]));
    printf("'user-name = first_name + last_name' vs 'user_profile = get_profile_data(user_id)': %.4f\n",
           array_direct_comparison(candidates[0], candidates[4]));
    printf("'user-name = first_name + last_name' vs 'customer_name = first_name + last_name': %.4f\n",
           array_direct_comparison(candidates[0], candidates[8]));
    
    return 0;
}