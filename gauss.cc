#include <algorithm>
#include <cctype>
#include "gauss.h"
#include <iostream>
#include <map>
#include <string>

// parse the string and return the code
// returns empty code on error
code_t parse_code(std::string s)
{
    code_t code; size_t length = s.length();
    for (int i = 0; i < length; ) {
        // can skip whitespace between elements
        if (std::isspace(s[i])) {
            i++; continue;
        }

        code_elem_t e = 0;
        if (s[i] == 'O') {
            e |= ELEM_OVER;
        } else if (s[i] != 'U') {
            return code_t();
        }

        if (++i >= length) return code_t();

        if (s[i] == '+') {
            e |= ELEM_POSITIVE;
        } else if (s[i] != '-') {
            return code_t();
        }

        if (++i >= length) return code_t();
        if (!std::isdigit(s[i])) return code_t();

        code_elem_t id = 0;
        while (std::isdigit(s[i])) {
            id = (id * 10) + (s[i++] - '0');
        }

        // todo: someday check for overflow, maybe
        e |= (id << ELEM_ID_SHIFT);
        code.push_back(e);
    }

    return first_ordered_code(code);
}

std::string stringify_code(code_t code)
{
    std::string s;
    for (int i = 0; i < code.size(); i++) {
        s.push_back(code[i] & ELEM_OU_MASK ? 'O' : 'U');
        s.push_back(code[i] & ELEM_SIGN_MASK ? '+' : '-');
        s += std::to_string(code[i] >> ELEM_ID_SHIFT);
    }

    return s;
}

void display_code(code_t code)
{
    std::cout << stringify_code(code) << std::endl;
}

void renumber_code(code_t &code, code_elem_t max_id)
{
    code_elem_t renum[max_id];
    for (size_t i = 0; i < max_id; i++) renum[i] = -1;

    size_t length = code.size();
    code_elem_t cur_max = 0;
    
    for (size_t i = 0; i < length; i++) {
        code_elem_t id = code[i] >> ELEM_ID_SHIFT;
        code_elem_t flags = code[i] & (ELEM_SIGN_MASK | ELEM_OU_MASK);

        if (renum[id] != -1) {
            code[i] = (renum[id] << ELEM_ID_SHIFT) | flags;
        } else {
            code[i] = (cur_max << ELEM_ID_SHIFT) | flags;
            renum[id] = cur_max++;
        }
    }
}

// takes in renumbered code
code_t rotate_code(code_t code, size_t num)
{
    // todo: can do this in one pass, measure impact later if it matters
    num %= code.size();
    if (!num) return code;

    std::rotate(code.begin(), code.begin() + num, code.end());
    renumber_code(code, code.size() / 2);
    return code;
}

// negative if a < b, positive if a > b, 0 if equal
int compare_codes(code_t a, code_t b)
{
    if (a.size() < b.size()) return -1;
    else if (a.size() > b.size()) return 1;

    for (int i = 0; i < a.size(); i++) {
        if (a[i] < b[i]) return -1;
        else if (a[i] > b[i]) return 1;
    }

    // equal
    return 0;
}

bool comparator_codes(code_t a, code_t b)
{
    if (a.size() < b.size()) return true;
    else if (a.size() > b.size()) return false;

    for (int i = 0; i < a.size(); i++) {
        if (a[i] < b[i]) return true;
        else if (a[i] > b[i]) return false;
    }

    // equal
    return false;
}

bool equal_codes(code_t a, code_t b)
{
    if (a.size() < b.size()) return false;
    else if (a.size() > b.size()) return false;

    for (int i = 0; i < a.size(); i++) {
        if (a[i] < b[i]) return false;
        else if (a[i] > b[i]) return false;
    }

    // equal
    return true;  
}

// pick the first of them based on the ordering from compare_codes
code_t first_ordered_code(code_t code)
{
    code_t first = code, prev;
    for (int i = 1; i < code.size(); i++) {
        prev = code;
        code = rotate_code(prev, 1);
        if (compare_codes(code, first) < 0) {
            first = code;
        }
    }

    return first;
}

code_t random_code(size_t max_length)
{
    // get a random code of max_length
    code_t c; //max_length = rand() % max_length;
    for (size_t i = 0; i < max_length; i++) {
        code_elem_t elem = (i << ELEM_ID_SHIFT);
        if (rand() % 2) elem |= ELEM_POSITIVE;
        c.push_back(elem); c.push_back(elem | ELEM_OVER);
    }

    std::random_shuffle(c.begin(), c.end());
    renumber_code(c, max_length);
    return first_ordered_code(c);
}
