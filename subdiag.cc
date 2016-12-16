#include <cassert>
#include "gauss.h"
#include "graph.h"
#include "subdiag.h"
#include <unordered_set>
#include <vector>

namespace std {
    template <>
    struct hash<code_t> {
        std::size_t operator()(const code_t &k) const
        {
            return std::hash<std::string>()(stringify_code(k));
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
static code_t remove_chords(code_t code, std::unordered_set<code_elem_t> set)
{
    code_t removed;
    for (size_t i = 0; i < code.size(); i++) {
        if (set.find(ELEM_ID(code[i])) != set.end()) {
            removed.push_back(code[i]);
        }
    }

    renumber_code(removed, code.size() / 2);
    return first_ordered_code(removed);
}

// get subdiagrams of a code
std::unordered_set<code_t> subdiagrams(code_t code)
{
    std::unordered_set<code_t> result;

    assert(code.size() / 2 <= MAX_CHORDS);

    auto lists = subsets[code.size() / 2];
    // each subset is associated with a list of what chords to remove
    for (size_t i = 0; i < lists.size(); i++) {
        result.insert(remove_chords(code, lists[i]));
    }

    return result;
}
