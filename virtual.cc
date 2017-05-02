#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include "gauss.h"
#include "genus.h"
#include <iostream>
#include <set>
#include <tuple>
#include <utility>

// takes in a gauss code and returns true if it's planar/classical
// the cubic version
bool planar_knot_cubic(const code_t& input_code)
{
    // complexity is: n^3, but there are other bottlenecks
    // from https://arxiv.org/pdf/math/0610929.pdf (which has a typo for S_j^{-1})
    size_t length = input_code.size();
    std::set<int> s[length / 2];
    std::vector<int> code;
    for (auto& iter: input_code) {
        // rewrite it so element O i preserves its sign, and U i flips its
        // also increment i by 1 so element 0 has a distinctive sign
        int elem = (ELEM_ID(iter) + 1) * SIGN(iter);
#ifndef FLAT_KNOTS
        if (!(iter & ELEM_OU_MASK)) {
            elem *= -1;
        }
#endif

        code.push_back(elem);
    }

    // construct S_i, the symbols between +i and -i in the cyclic
    // gauss code
    for (int i = 0; i < length / 2; i++) {
        int idx;
        for (idx = 0; idx < length; idx++) {
            if (code[idx] == i + 1) {
                // found the positive one
                idx = (idx + 1) % length; break;
            }
        }

        while (std::abs(code[idx]) != i + 1) {
            s[i].insert(code[idx]);
            idx = (idx + 1) % length;
        }
    }

    // the sum of signs in S_i must be 0
    for (int i = 0; i < length / 2; i++) {
        int sum = 0;
        for (auto iter: s[i]) {
            sum += (iter > 0) ? +1 : -1; 
        }

        if (sum) {
            return false;
        }
    }

    // the sum of signs in (S_i \cup {+i, -i}) \cap (S_j^{-1})
    // must be zero, where S_j^{-1} has all superscripts reversed
    for (int i = 0; i < length / 2; i++) {
        for (int j = 0; j < length / 2; j++) {
            std::set<int> temp_s_i = s[i];
            // S_i \cup {+i, -i}
            temp_s_i.insert(i + 1); temp_s_i.insert(-(i + 1));

            // S_j^{-1}
            std::set<int> temp_s_j;
            for (auto iter: s[j]) {
                temp_s_j.insert(iter * -1);
            }

            // their intersection
            std::vector<int> intersection;
            std::set_intersection(temp_s_i.begin(), temp_s_i.end(),
                                  temp_s_j.begin(), temp_s_j.end(),
                                  std::back_inserter(intersection));

            int sum = 0;
            for (auto iter: intersection) {
                sum += (iter > 0) ? +1 : -1;
            }

            if (sum) {
                return false;
            }
        }
    }

    return true;
}

// takes in a gauss code and returns true if it's planar/classical
bool planar_knot(const code_t& code)
{
#ifndef FLAT_KNOTS
    return (genus(code) == 0);
#else
    return planar_knot_cubic(code);
#endif
}
