#ifndef _GAUSS_H
#define _GAUSS_H

#include <string>
#include <vector>

typedef unsigned int code_elem_t;

// 1 -> positive, 0 -> negative
#define ELEM_SIGN_MASK  (1 << 0)
#define ELEM_POSITIVE   (1 << 0)

#define SIGN(x)         (((x) & ELEM_POSITIVE) ? +1 : -1)

// 1 -> O, 0 -> U
#define ELEM_OU_MASK    (1 << 1)
#define ELEM_OVER       (1 << 1)

#define OVER(x)         ((x) & ELEM_OVER)

#define ELEM_ID_MASK    ~(ELEM_SIGN_MASK | ELEM_OU_MASK)
#define ELEM_ID_SHIFT   2

#define ELEM_ID(x)      ((x) >> ELEM_ID_SHIFT)

typedef std::vector<code_elem_t> code_t;

// parse the string and return the code
code_t parse_code(std::string s);

std::string stringify_code(code_t code);
void display_code(code_t code);

void renumber_code(code_t &code, code_elem_t max_id);
code_t first_ordered_code(code_t code);

// negative if a < b, positive if a > b, 0 if equal
int compare_codes(code_t a, code_t b);
bool comparator_codes(code_t a, code_t b);
bool equal_codes(code_t a, code_t b);

code_t random_code(size_t max_length);

#endif /* _GAUSS_H */
