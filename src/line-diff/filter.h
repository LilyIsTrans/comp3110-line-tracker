#pragma once
#include "distance.h"
#include "context.h"
#include "../string-slice/string-slice.h" // note: have to link this when compiling based on my testing
#include <stdio.h>
#include <inttypes.h>

#define MAX_CANDIDATES 15

uint32_t simhash(Substring *str);

int hammingDistance(uint32_t a, uint32_t b);

struct SubstringArray* filterCandidates(Substring *str, struct SubstringArray *candidates);

int main(int argc, const char **argv);