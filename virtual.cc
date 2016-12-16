#include <algorithm>
#include <cmath>
#include "gauss.h"
#include <iostream>
#include <set>

// takes in a gauss code and returns true if it's planar/classical
bool planar_knot(code_t input_code)
{
    // todo: look at better implementations, since this can be done
    // in linear time?
    // current complexity is: n^3, but there are other bottlenecks
    // from https://arxiv.org/pdf/math/0610929.pdf (which has a typo for S_j^{-1})
    std::set<int> s[input_code.size() / 2];
    std::vector<int> code;
    for (int i = 0; i < input_code.size(); i++) {
        // rewrite it so element O i preserves its sign, and U i flips its
        // also increment i by 1 so element 0 has a distinctive sign
        int elem = (input_code[i] >> ELEM_ID_SHIFT) + 1;
        if (input_code[i] & ELEM_OU_MASK) {
            elem *= (input_code[i] & ELEM_SIGN_MASK) ? +1 : -1;
        } else {
            elem *= (input_code[i] & ELEM_SIGN_MASK) ? -1 : +1;
        }

        code.push_back(elem);
    }

    // construct S_i, the symbols between +i and -i in the cyclic
    // gauss code
    for (int i = 0; i < code.size() / 2; i++) {
        int idx;
        for (idx = 0; idx < code.size(); idx++) {
            if (code[idx] == i + 1) {
                // found the positive one
                idx = (idx + 1) % code.size(); break;
            }
        }

        while (std::abs(code[idx]) != i + 1) {
            s[i].insert(code[idx]);
            idx = (idx + 1) % code.size();
        }
    }

    // the sum of signs in S_i must be 0
    for (int i = 0; i < code.size() / 2; i++) {
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
    for (int i = 0; i < code.size() / 2; i++) {
        for (int j = 0; j < code.size() / 2; j++) {
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
