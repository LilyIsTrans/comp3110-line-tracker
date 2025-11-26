#include "../../src/string-slice/string-slice.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//split a SubstringArray of lines into tokens based on delimiter
struct SubstringArray* tokenize_lines(struct SubstringArray* lines, char delimiter) {

	struct SubstringArray* output = new_substring_array(0);  //new substring array to hold tokens

	//iterate through lines
	for (size_t i = 0; i < lines->len; i++){
		struct SubstringArray* line_tokens = malloc(sizeof(struct SubstringArray));
		line_tokens = tokenize(lines->array[i], delimiter); //tokenize current line

		//append tokens from current line to output array
		for (size_t j = 0; j < line_tokens->len; j++){
			output = append_to_substring_array(output, line_tokens->array[j]);
		}
		free(line_tokens); //free memory allocated for line tokens
	}
	return output;
}

//split a substring into tokens based on delimiter
struct SubstringArray* tokenize(Substring haystack, char delimiter) {

	struct SubstringArray* output = new_substring_array(0);  //new substring array to hold tokens
	Substring last_token_so_far = haystack;
	last_token_so_far.end = last_token_so_far.start;

	//iterate through haystack to find tokens
	while (last_token_so_far.end < haystack.end) {
		if (*last_token_so_far.end == delimiter) {
			output = append_to_substring_array(output, last_token_so_far);   //append substring up to delimiter to output array
			last_token_so_far.start = last_token_so_far.end + 1;			 //update start pointer to character after delimiter
		}
		last_token_so_far.end++;
	}

	//append final token after last delimiter (or whole string if no delimiters found)
	output = append_to_substring_array(output, last_token_so_far);
	return output;
}

//if SubstringArray contains program keywords identify which ones
struct HashArray* identify_keywords(struct SubstringArray* tokens, struct SubstringArray* keywords) {
	struct HashArray* output = new_hash_array(tokens->len); //new hash array to hold keyword IDs

	//iterate through tokens
	for (size_t i = 0; i < tokens->len; i++){
		bool found_keyword = false;

		//check if token matches any keyword
		for (size_t j = 0; j < keywords->len; j++) {
			if (tokens->array[i].end - tokens->array[i].start == keywords->array[j].end - keywords->array[j].start &&
				strncmp(tokens->array[i].start, keywords->array[j].start, tokens->array[i].end - tokens->array[i].start) == 0) {
				output->array[i] = (uint32_t)j + 1; //store keyword ID (1-based index)
				found_keyword = true;
				break;
			}
		}

		if (!found_keyword) {
			output->array[i] = 0; //not a keyword
		}
	}

	output->len = tokens->len;
	return output;
}

//if substring is a keyword, return ID (otherwise return 0)
uint32_t get_keyword_id(Substring token, struct SubstringArray* keywords) {

	//iterate through keywords
	for (size_t j = 0; j < keywords->len; j++) {

		if (token.end - token.start == keywords->array[j].end - keywords->array[j].start &&
			strncmp(token.start, keywords->array[j].start, token.end - token.start) == 0) {
			return (uint32_t)j + 1; //return keyword ID
		}
	}
	return 0;
}

//normalize tokens to lowercase
struct SubstringArray* normalize_tokens_lowercase(struct SubstringArray* tokens) {

	for (size_t i = 0; i < tokens->len; i++) {
		for (const char* p = tokens->array[i].start; p < tokens->array[i].end; p++) {
			if (*p >= 'A' && *p <= 'Z') {
				//convert to lowercase
				*(char*)p = *p + ('a' - 'A');
			}
		}
	}
	return tokens;
}

//normalize all token variables in SubstringArray
struct SubstringArray* normalize_variable_names(struct SubstringArray* tokens, struct SubstringArray* keywords) {
	
	//array to hold mappings from original variable names to normalized characters
	struct SubstringArray* variable_mappings = new_substring_array(0);

	//start with 'a' for first variable name
	char current_var_char = 'a';

	for (size_t i = 0; i < tokens->len; i++) {
		//if not a keyword
		if (get_keyword_id(tokens->array[i], keywords) == 0) {
			//check if this variable name has already been mapped
			bool found_mapping = false;

			for (size_t j = 0; j < variable_mappings->len; j++) {
				if (length(tokens->array[i]) == length(variable_mappings->array[j]) &&
					strncmp(tokens->array[i].start, variable_mappings->array[j].start, length(tokens->array[i])) == 0) {

					//mapping found, use the corresponding normalized character
					char mapped_char = *(variable_mappings->array[j].end - 1);

					for (const char* p = tokens->array[i].start; p < tokens->array[i].end; p++) {
						if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || (*p == '_')) {
							*(char*)p = mapped_char; //normalize variable name character to mapped character
						}
					}
					found_mapping = true;
					break;
				}
			}
			//if no mapping found, create a new one
			if (!found_mapping) {

				//create mapping substring
				Substring mapping;
				mapping.start = tokens->array[i].start;
				mapping.end = tokens->array[i].end;

				//append mapping to variable_mappings array with current_var_char as the normalized character
				variable_mappings = append_to_substring_array(variable_mappings, mapping);
				*(char*)(variable_mappings->array[variable_mappings->len - 1].end - 1) = current_var_char;

				//normalize current token with current_var_char
				for (const char* p = tokens->array[i].start; p < tokens->array[i].end; p++) {
					if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || (*p == '_')) {
						*(char*)p = current_var_char; //normalize variable name character to current_var_char
					}
				}
				//increment to next character for next variable name
				if (current_var_char == 'z') {
					current_var_char = 'A';		//wrap around to 'A' after 'z'
				}
				else if (current_var_char == 'Z') {
					current_var_char = 'a';		//wrap around to 'a' after 'Z'
				}
				else {
					current_var_char++;
				}
			}
		}
	}
	free(variable_mappings); //free memory allocated for variable mappings
	return tokens;
}
