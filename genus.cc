#include <cassert>
#include "gauss.h"
#include <set>
#include <tuple>

enum sign_t {
    POSITIVE = +1, NONE = 0, NEGATIVE = -1
};

typedef struct vertex {
    int i;
    sign_t sign;

    bool operator <(const struct vertex& b) const
    {
        return std::tie(i, sign) < std::tie(b.i, b.sign);
    }

    bool operator !=(const struct vertex& b) const
    {
        return std::tie(i, sign) != std::tie(b.i, b.sign);
    }

    bool operator ==(const struct vertex& b) const
    {
        return std::tie(i, sign) == std::tie(b.i, b.sign);
    }
} vertex_t;

typedef struct edge {
    vertex_t a;
    vertex_t b;
    sign_t sign;

    bool operator <(const struct edge& rhs) const
    {
        return std::tie(a, b, sign) < std::tie(rhs.a, rhs.b, rhs.sign);
    }

    bool operator !=(const struct edge& rhs) const
    {
        return std::tie(a, b, sign) != std::tie(rhs.a, rhs.b, rhs.sign);
    }

    bool operator ==(const struct edge& rhs) const
    {
        return std::tie(a, b, sign) == std::tie(rhs.a, rhs.b, rhs.sign);
    }
} edge_t;

// takes in a gauss code and returns true if it's planar/classical
int genus(const code_t& input_code)
{
    // return the genus of the diagram
    // from https://arxiv.org/pdf/math/0610929.pdf
    size_t length = input_code.size(), vertices = length / 2;
    if (!length) {
        return 0;
    }

    std::vector<vertex_t> code;
    std::set<edge_t> free_edges;
    std::vector<edge_t> edges_starting_with[vertices];

    for (auto& iter: input_code) {
        // rewrite it so element U i preserves its sign, and O i is just
        // the integer
        // also increment i by 1 so element 0 has a distinctive sign
        vertex_t v; v.i = ELEM_ID(iter);
        if (!(iter & ELEM_OU_MASK)) {
            v.sign = (sign_t) SIGN(iter);
        } else {
            v.sign = NONE;
        }

        code.push_back(v);
    }

    for (int i = 0; i < length; i++) {
        vertex_t& first = code[i], second = code[(i + 1) % length];
        edge_t forward; forward.a = first; forward.b = second; forward.sign = POSITIVE;
        free_edges.insert(forward);
        edges_starting_with[first.i].push_back(forward);

        edge_t reverse; reverse.a = second; reverse.b = first; reverse.sign = NEGATIVE;
        free_edges.insert(reverse);
        edges_starting_with[second.i].push_back(reverse);
    }

    size_t faces = 0;
    while (!free_edges.empty()) {
        // get a cycle
        auto e_iter = free_edges.begin();
        auto e = *e_iter;
        auto end = *e_iter; auto start = e;

        do {
            // loop around
            sign_t e_sign = end.sign;

            // turn "right"
            if (end.b.sign == NONE) {
                sign_t factor = (end.sign == NEGATIVE) ? NEGATIVE : POSITIVE;

                // for edges starting with `end`
                for (auto& iter: edges_starting_with[end.b.i]) {
                    if (iter.a.sign == factor * iter.sign) {
                        end = iter; break;
                    }
                }
            } else {
                sign_t factor = (end.b.sign == POSITIVE) ? NEGATIVE : POSITIVE;

                for (auto& iter: edges_starting_with[end.b.i]) {
                    if (iter.a.sign == NONE && iter.sign == factor * end.sign) {
                        end = iter; break;
                    }
                }
            }

            auto iter = free_edges.find(end); assert(iter != free_edges.end());
            free_edges.erase(iter);
        } while (end != start);

        faces++;
    }

    return (2 - (faces - vertices)) / 2;
}
