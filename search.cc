#include <cassert>
#include "gauss.h"
#include "graph.h"
#include <iostream>
#include "moves.h"
#include <string>
#include "subdiag.h"
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "virtual.h"

namespace std {
    template <>
    struct hash<code_t> {
        size_t operator()(const code_t& k) const
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

std::unordered_map<code_t, node_t*> graph_nodes;
std::unordered_set<node_t*> prune_dirty;

static node_t* get_node(const code_t& code)
{
    node_t *node;
    auto iter = graph_nodes.find(code);
    if (iter == graph_nodes.end()) {
        // not found, so we need to create the node
        node = new node_t;
        node->code = code;
        node->sify = node->pruneify = false;
        node->neighbors_explored = node->sneighbors_explored = false;

        node->planar = planar_knot(code);

        graph_nodes[code] = node;
    } else {
        node = iter->second;
    }

    return node;
}

static bool is_planar(const code_t& code)
{
    // todo: measure impact of this
    return (get_node(code))->planar;
}

static void add_neighbors(node_t *node, std::vector<code_t>& neighbors)
{
    for (auto& iter: neighbors) {
        node_t *neigh = get_node(iter);
        if (node->neighbors.find(neigh) == node->neighbors.end()) {
            node->neighbors.insert(neigh);
            neigh->neighbors.insert(node);
        }       
    }
}

// explores all the special neighbors of a node
static void explore_special_neighbors(node_t* node)
{
    if (node->sneighbors_explored) {
        return;
    }

    auto neighbors = enumerate_special_neighbors(node->code);
    add_neighbors(node, neighbors);

    node->sneighbors_explored = true;
}

// explores all the neighbors of a node
static void explore_complete_neighbors(node_t* node)
{
    if (node->neighbors_explored) {
        return;
    }

    std::vector<code_t> neighbors;
    if (node->sneighbors_explored) {
        neighbors = enumerate_rest_neighbors(node->code);
    } else {
        neighbors = enumerate_complete_neighbors(node->code);
    }

    add_neighbors(node, neighbors);
    node->neighbors_explored = node->sneighbors_explored = true;
}

// make a node 's'ed
static node_t* s_ify(node_t* node)
{
    if (node->code.size() > MAX_CHORDS * 2 ||
        node->sify) {
        return node;
    }

    if (node->planar) {
        // if it's planar, it has to map to itself
        node->s.insert(node);
        explore_special_neighbors(node);
    } else {
        // otherwise all classical subdiagrams are a possibility
        auto subdiags = subdiagrams(node->code);
        for (auto iter: subdiags) {
            node_t *sub = get_node(iter);
            if (sub->planar) {
                explore_special_neighbors(sub);
                node->s.insert(sub);
            }
        }
    }

    node->sify = true;
    return node;
}

node_t* s_ify(const code_t& code)
{
    node_t* node = get_node(code);
    return s_ify(node);
}

// make a node that's going to be pruned
node_t* prune_ify(node_t* node)
{
    if (node->code.size() > MAX_CHORDS * 2 ||
        node->pruneify) {
        return node;
    }

    s_ify(node);

    // a more exhaustive search
    // todo: remove two layers and this part if unneeded
    explore_special_neighbors(node);
    // // all neighbors need to be 's'ed
    for (auto iter: node->neighbors) {
        s_ify(iter->code);
    }

    // insert in set of nodes to be pruned
    prune_dirty.insert(node);
    node->pruneify = true;
    return node;
}

node_t* prune_ify(const code_t& code)
{
    node_t* node = get_node(code);
    return prune_ify(node);
}

// check if e is a plausible erasure for d
bool plausible_erasure(node_t *e, node_t *d)
{
    assert(d->pruneify);

    for (auto n: d->neighbors) {
        // for every neighbor n of d
        if (!n->sify) {
            // if we haven't generated S set, can't check if it's valid
            continue;
        }

        bool found = false;

        // there is some f in s(n) that is either e or one step away
        // from it
        for (auto f: n->s) {
            assert(f->sneighbors_explored && e->sneighbors_explored);
            if ((f == e) || (f->neighbors.find(e) != f->neighbors.end())) {
                found = true;
                break;
            }
        }

        // if we couldn't find any such element, not plausible
        if (!found) {
            // std::cout << "Erasing " << stringify_code(e->code) << " from " << stringify_code(d->code)
            //         << " because of "; display_code(n->code);
            return false;
        }
    }

    return true;
}

// prune the prune-ified elements, returning false if it's an
// implausible scenario
bool prune()
{
    while (!prune_dirty.empty()) {
        auto d_iter = prune_dirty.begin();
        auto d = *d_iter; prune_dirty.erase(d_iter);

        bool deleted = false;
        for (auto e: d->s) {
            if (!plausible_erasure(e, d)) {
                d->s.erase(e);
                deleted = true;
            }
        }

        // if set is empty, implausible scenario
        if (d->s.empty()) {
            std::cout << "The contradiction came with "; display_code(d->code);
            return false;
        }

        // if we deleted something, need to add all neighbors as
        // dirty
        if (deleted) {
            for (auto n: d->neighbors) {
                if (n->pruneify) {
                    prune_dirty.insert(n);
                }
            }
        }
    }

    return true;
}

void explore()
{
    // add the unknot
    node_t* node = prune_ify(code_t());

    // std::cout << node->s.size() << " subdiagram(s) of "; display_code(node->code);
    // for (auto s_elem: node->s) {
    //     display_code(s_elem->code);
    // }
    // std::cout << "-----------" << std::endl;

    explore_complete_neighbors(node);
    for (auto n: node->neighbors) {
        prune_ify(n);
        explore_complete_neighbors(n);
        for (auto nn: n->neighbors) {
            prune_ify(nn);
            explore_complete_neighbors(nn);
            for (auto nnn: nn->neighbors) {
                prune_ify(nnn);
                explore_special_neighbors(nnn);
                for (auto nnnn: nnn->neighbors) {
                    prune_ify(nnnn);
                    break;
                }
            }
        }
    }

    // for (auto iter: prune_dirty) {
    //     std::cout << iter->s.size() << " subdiagram(s) of "; display_code(iter->code);
    //     for (auto s_elem: iter->s) {
    //         display_code(s_elem->code);
    //     }
    //     std::cout << "-----------" << std::endl;

    //     for (auto n: iter->neighbors) {
    //         if (prune_dirty.find(n) != prune_dirty.end()) continue;
    //         std::cout << n->s.size() << " subdiagram(s) of " << stringify_code(n->code)
    //                   << " neighbor of "; display_code(iter->code);
    //         for (auto s_elem: n->s) {
    //             display_code(s_elem->code);
    //         }
    //         std::cout << "-----------" << std::endl;
    // }

    // std::cout << "Pruning now" << std::endl;
    std::cout << "Size of prune set is " << prune_dirty.size() << std::endl;
    prune();

    // std::cout << "-----------" << std::endl;
    // std::cout << node->s.size() << " subdiagram(s) of "; display_code(node->code);
    // for (auto s_elem: node->s) {
    //     display_code(s_elem->code);
    // }
    // std::cout << "-----------" << std::endl;

    // for (auto n: node->neighbors) {
    //     std::cout << n->s.size() << " subdiagram(s) of "; display_code(n->code);
    //     for (auto s_elem: n->s) {
    //         display_code(s_elem->code);
    //     }
    //     std::cout << "-----------" << std::endl;
    // }
}
