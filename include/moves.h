#ifndef _MOVES_H
#define _MOVES_H

#include "gauss.h"

std::vector<code_t> r1_undo_enumerate(code_t code);
std::vector<code_t> r1_do_enumerate(code_t code);
std::vector<code_t> r2_undo_enumerate(code_t code);
std::vector<code_t> r2_do_enumerate(code_t code);
std::vector<code_t> r3_enumerate(code_t code);

// enumerate neighbors of code
std::vector<code_t> enumerate_neighbors(code_t code);

#endif /* _MOVES_H */
