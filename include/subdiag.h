#ifndef _SUBDIAG_H
#define _SUBDIAG_H

#include "gauss.h"
#include <unordered_set>

// initialize the a list of subsets
void subsets_init();

// get subdiagrams of a code
std::unordered_set<code_t> subdiagrams(code_t code);

#endif /* _SUBDIAG_H */
