#include <cassert>
#include "gauss.h"
#include "graph.h"
#include <iostream>
#include "moves.h"
#include <set>
#include <string>
#include "subdiag.h"
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "virtual.h"

namespace std {
    template <>
    struct hash<code_t> {
        std::size_t operator()(const code_t &k) const
        {
            return std::hash<std::string>()(stringify_code(k));
        }
    };
}

std::unordered_map<code_t, node_t*> graph_nodes;
std::unordered_set<node_t*> prune_dirty;

static node_t* get_node(code_t code)
{
    node_t *node;
    auto iter = graph_nodes.find(code);
    if (iter == graph_nodes.end()) {
        // not found, so we need to create the node
        node = new node_t;
        node->code = code;
        node->sify = node->pruneify = node->neighbors_explored = false;
        node->planar = planar_knot(code);
        graph_nodes[code] = node;
    } else {
        node = iter->second;
    }

    return node;
}

static bool is_planar(node_t *node)
{
    return node->planar;
}

static bool is_planar(code_t code)
{
    return (get_node(code))->planar;
}

// explores all the neighbors of a node
static void explore_neighbors(node_t* node)
{
    if (node->neighbors_explored) {
        return;
    }

    auto neighbors = enumerate_neighbors(node->code);
    for (auto iter: neighbors) {
        node_t *neigh = get_node(iter);
        if (node->neighbors.find(neigh) == node->neighbors.end()) {
            node->neighbors.insert(neigh);
            neigh->neighbors.insert(node);
        }
    }

    node->neighbors_explored = true;
}

// make a node 's'ed
static node_t* s_ify(node_t *node)
{
    if (node->code.size() > MAX_CHORDS * 2 ||
        node->sify) {
        return node;
    }

    if (planar_knot(node->code)) {
        // if it's planar, it has to map to itself
        node->s.insert(node);
        explore_neighbors(node);
    } else {
        // otherwise all classical subdiagrams are a possibility
        auto subdiags = subdiagrams(node->code);
        for (auto iter: subdiags) {
            node_t *sub = get_node(iter);
            if (is_planar(sub)) {
                explore_neighbors(sub);
                node->s.insert(sub);
            }
        }
    }

    node->sify = true;
    return node;
}

node_t* s_ify(code_t code)
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
    // explore_neighbors(node);
    // all neighbors need to be 's'ed
    // for (auto iter: node->neighbors) {
    //     s_ify(iter->code);
    // }

    // insert in set of nodes to be pruned
    prune_dirty.insert(node);
    node->pruneify = true;
    return node;
}

node_t* prune_ify(code_t code)
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
            // std::cout << "f "; display_code(f->code);
            // std::cout << "n "; display_code(n->code);
            // std::cout << "d "; display_code(d->code);
            assert(f->neighbors_explored);
            if ((f == e) || (f->neighbors.find(e) != f->neighbors.end())) {
                found = true;
                break;
            }
        }

        // if we couldn't find any such element, not plausible
        if (!found) {
            // std::cout << "The pesky neighbor was "; display_code(n->code);
            // std::cout << "Enumerating S" << std::endl;
            // for (auto f: n->s) {
            //     display_code(f->code);
            // }
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
        // start all over after deleting an implausible erasure
        // because s(n) changes, so some older element might not
        // be plausible anymore
        // std::cout << "GOING OVER "; display_code(d->code);
        // if set is empty, implausible scenario
        for (auto e: d->s) {
            if (!plausible_erasure(e, d)) {
                d->s.erase(e);
                deleted = true;
            }
        }

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
}

void explore()
{
    // add the unknot
    node_t* node = prune_ify(code_t());
    // node_t* node = prune_ify(parse_code("O+0U+1O+2U+0O+1U+2"));

    // todo: fix this eventually to something standard, but tinkering
    // right now
    explore_neighbors(node);
    for (auto n: node->neighbors) {
        prune_ify(n);
        explore_neighbors(n);
        for (auto nn: n->neighbors) {
            prune_ify(nn);
            explore_neighbors(nn);
            for (auto nnn: nn->neighbors) {
                prune_ify(nnn);
            }
        }
    }
                
    std::cout << "Size of prune set is " << prune_dirty.size() << std::endl;
    std::cout << "Pruning finished with " << (prune() ? "success" : "failure") << std::endl;
}
