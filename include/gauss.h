#ifndef _GAUSS_H
#define _GAUSS_H

#include <string>
#include <vector>

// #define FLAT_KNOTS

typedef unsigned int code_elem_t;

// 1 -> positive, 0 -> negative
#define ELEM_SIGN_MASK  (1 << 0)
#define ELEM_POSITIVE   (1 << 0)

#define SIGN(x)         (((x) & ELEM_POSITIVE) ? +1 : -1)

#ifdef FLAT_KNOTS

#define ELEM_FLAGS_MASK	(ELEM_SIGN_MASK)
#define ELEM_ID_MASK	(~ELEM_FLAGS_MASK)
#define ELEM_ID_SHIFT	1

#else

// 1 -> O, 0 -> U
#define ELEM_OU_MASK    (1 << 1)
#define ELEM_OVER       (1 << 1)

#define OVER(x)         ((x) & ELEM_OVER)

#define ELEM_FLAGS_MASK (ELEM_SIGN_MASK | ELEM_OU_MASK)
#define ELEM_ID_MASK    (~ELEM_FLAGS_MASK)
#define ELEM_ID_SHIFT   2

#endif

#define ELEM_ID(x)      ((x) >> ELEM_ID_SHIFT)

typedef std::vector<code_elem_t> code_t;

// parse the string and return the code
code_t parse_code(const std::string& s);

std::string stringify_code(const code_t& code);
void display_code(const code_t& code);

void renumber_code(code_t &code, code_elem_t max_id);
code_t first_ordered_code(code_t code);

// negative if a < b, positive if a > b, 0 if equal
int compare_codes(const code_t& a, const code_t& b);

code_t random_code(size_t max_length);

#endif /* _GAUSS_H */
