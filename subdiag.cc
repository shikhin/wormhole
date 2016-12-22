#include <cassert>
#include "gauss.h"
#include "graph.h"
#include "subdiag.h"
#include <unordered_set>
#include <vector>

namespace std {
    template <>
    struct hash<code_t> {
        size_t operator()(const code_t &k) const
        {
            // return std::hash<std::string>()(stringify_code(k));
            // stolen from http://stackoverflow.com/questions/20511347/a-good-hash-function-for-a-vector
            code_elem_t seed = k.size();
            for (auto i: k) {
                seed ^= i + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}

std::vector< std::unordered_set<code_elem_t> > subsets[MAX_CHORDS + 1];

// initialize the a list of subsets
void subsets_init()
{
    subsets[0].push_back(std::unordered_set<code_elem_t>());
    for (size_t i = 1; i < MAX_CHORDS + 1; i++) {
        subsets[i] = subsets[i - 1];
        for (auto iter: subsets[i - 1]) {
            auto set = iter; set.insert(i - 1);
            subsets[i].push_back(set);
        }
    }
}

// std::vector< std::unordered_set<code_elem_t> > subsets_make(std::unordered_set<code_elem_t> set)
// {
//     std::vector< std::unordered_set<code_elem_t> > result;
//     if (set.empty()) {
//         result.push_back(set);
//         return result;
//     }

//     std::unordered_set<code_elem_t> almost = set;
//     code_elem_t first_val = *almost.begin();
//     almost.erase(almost.begin());

//     // exclude the last
//     auto half = subsets_make(almost);
//     result.insert(result.end(), half.begin(), half.end());

//     // add the last
//     for (size_t i = 0; i < half.size(); i++) {
//         half[i].insert(first_val);
//         result.push_back(half[i]);
//     }

//     return result;
// }

// remove chords from the code based on set
static code_t remove_chords(const code_t &code, const std::unordered_set<code_elem_t> &set)
{
    code_t removed;
    for (auto& iter: code) {
        if (set.find(ELEM_ID(iter)) != set.end()) {
            removed.push_back(iter);
        }
    }

    renumber_code(removed, code.size() / 2);
    return removed;
}

// get subdiagrams of a code
std::unordered_set<code_t> subdiagrams(const code_t& code)
{
    std::unordered_set<code_t> result;

    assert(code.size() / 2 <= MAX_CHORDS);

    auto &lists = subsets[code.size() / 2];
    // each subset is associated with a list of what chords to remove
    for (auto& iter: lists) {
        result.insert(remove_chords(code, iter));
    }

    // order them after removing duplicates one
    std::unordered_set<code_t> result_ordered;
    for (auto& iter: result) {
        result_ordered.insert(first_ordered_code(iter));
    }

    return result_ordered;
}
