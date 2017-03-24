#ifndef _MOVES_H
#define _MOVES_H

#include "gauss.h"

std::vector<code_t> r1_undo_enumerate(const code_t& code);
std::vector<code_t> r1_do_enumerate(const code_t& code);
std::vector<code_t> r2_undo_enumerate(const code_t& code);
std::vector<code_t> r2_do_enumerate(const code_t& code);
std::vector<code_t> r3_enumerate(const code_t& code);

// enumerate neighbors of code
std::vector<code_t> enumerate_complete_neighbors(const code_t& code);

// enumerate neighbors of code such that if enumerated on X and Y, they'll
// be connected if they can be
std::vector<code_t> enumerate_special_neighbors(const code_t& code);

// enumerates neighbors not enumerated by rest
std::vector<code_t> enumerate_nonspecial_neighbors(const code_t& code);

// enumerate neighbors of code (unsan)
std::vector<code_t> enumerate_complete_unsan_neighbors(const code_t& code);

// cleans up a movie so the unsanitary versions are the ones that are displayed
// yes, some people like unsanitary movies
void cleanup_movie(std::vector<std::string> movie);

#endif /* _MOVES_H */
