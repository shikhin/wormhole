#include <cassert>
#include "gauss.h"
#include "genus.h"
#include "graph.h"
#include <iostream>
#include <limits>
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

#ifdef TEST_BRUTE
static const size_t max_indices = 4000;
static size_t cur_index = 0, not_added = 0;

static node_t* node_table[max_indices];

static size_t classical_dist[max_indices][max_indices], virtual_dist[max_indices][max_indices];

static void brute_insert_node(node_t* node);
#endif

static std::unordered_map<code_t, node_t*> graph_nodes;
static std::unordered_set<node_t*> prune_dirty;

#ifdef TEST_MENU
static std::unordered_set<node_t*> menuable_nodes;
#endif

static const size_t chunk_size = 10000;
static size_t chunk_idx = 0;
static node_t* chunk = NULL;

static node_t* alloc_node()
{
    if (chunk_idx >= chunk_size) {
        chunk = new node_t[chunk_size];
        chunk_idx = 0;
    }

    return &chunk[chunk_idx++];
}

static node_t* get_node(const code_t& code)
{
    node_t *node;
    auto iter = graph_nodes.find(code);
    if (iter == graph_nodes.end()) {
        // not found, so we need to create the node
        node = alloc_node();
        node->code = code;
        node->pruneify = false;
        node->neighbors_explored = node->sneighbors_explored = false;
#ifdef TEST_BRUTE
        node->index = max_indices;
#endif
        node->genus = genus(code);

        graph_nodes[code] = node;
    } else {
        node = iter->second;
    }

    return node;
}

static bool is_planar(const node_t* node)
{
    return (node->genus <= 0);
}

static bool is_planar(const code_t& code)
{
    return is_planar(get_node(code));
}

static void add_neighbors(node_t *node, std::vector<code_t>& neighbors)
{
    for (auto& iter: neighbors) {
        node_t *neigh = get_node(iter);
        node->neighbors.insert(neigh);
        neigh->neighbors.insert(node);
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
        neighbors = enumerate_nonspecial_neighbors(node->code);
    } else {
        neighbors = enumerate_complete_neighbors(node->code);
    }

    add_neighbors(node, neighbors);
    node->neighbors_explored = node->sneighbors_explored = true;
}

static void generate_subs(node_t* node)
{
    if (!node->subs.empty()) {
        return;
    }

    // generate all classical subdiagrams
    auto subdiags = subdiagrams(node->code);
    for (auto iter: subdiags) {
        node_t *sub = get_node(iter);
        if (is_planar(sub)) {
            explore_special_neighbors(sub);
            node->subs.insert(sub);
        }
    }
}

static void add_r3_neighborhood_subs(node_t* node, std::set<node_t*>& seen, node_t* cur)
{
    auto cur_r3_neighbors = r3_enumerate(cur->code);
    for (auto iter: cur_r3_neighbors) {
        auto n = get_node(iter); generate_subs(n);
        // if we've seen this one, don't explore it again
        if (seen.find(n) != seen.end()) {
            continue;
        }

        // mark it as seen and steal its subs
        seen.insert(n);
        node->s.insert(n->subs.begin(), n->subs.end());

        // recurse
        add_r3_neighborhood_subs(node, seen, n);
    }
}

// make a node 's'ed
static node_t* s_ify(node_t* node)
{
    if (!node->s.empty()) {
        return node;
    }

#ifdef TEST_MENU
    menuable_nodes.insert(node);
#endif

#ifdef TEST_BRUTE
    brute_insert_node(node);
#endif

    generate_subs(node);

    if (is_planar(node)) {
        // if it's planar, it has to map to itself
        node->s.insert(node);
    } else {
        // otherwise all classical subdiagrams are a possibility
        node->s = node->subs;
    }

#ifdef TEST_R3_UNIFY
    std::set<node_t*> seen; seen.insert(node);
    if (!is_planar(node)) {
        // get the complete r3 neighborhood for non-planar nodes
        add_r3_neighborhood_subs(node, seen, node);
    }
#endif

    return node;
}

static node_t* s_ify(const code_t& code)
{
    node_t* node = get_node(code);
    return s_ify(node);
}

// make a node that's going to be pruned
static node_t* prune_ify(node_t* node)
{
    if (node->pruneify) {
        return node;
    }

    s_ify(node);

    // a more exhaustive search
    // explore_special_neighbors(node);
    // // all neighbors need to be 's'ed
    // for (auto iter: node->neighbors) {
// #ifdef TEST_BRUTE
        // brute_insert_node(iter);
// #endif
        // s_ify(iter->code);
    // }

    // insert in set of nodes to be pruned
    prune_dirty.insert(node);
    node->pruneify = true;

    return node;
}

static node_t* prune_ify(const code_t& code)
{
    node_t* node = get_node(code);
    return prune_ify(node);
}

// check if e is a plausible erasure for d
static bool plausible_erasure(node_t *e, node_t *d)
{
    assert(d->pruneify);

    for (auto n: d->neighbors) {
        // for every neighbor n of d
        if (n->s.empty()) {
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
static bool test_hillary()
{
    while (!prune_dirty.empty()) {
        auto d_iter = prune_dirty.begin();
        auto d = *d_iter; prune_dirty.erase(d_iter);

        bool deleted = false; auto iter = d->s.begin();
        while (iter != d->s.end()) {
            auto current = iter++;
            auto e = *current;
            if (!plausible_erasure(e, d)) {
                d->s.erase(current);
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

#ifdef TEST_MENU
// test if the current set of menuable nodes is actually menuable
static void test_menu()
{
    for (auto n: menuable_nodes) {
        // where you started from
        menu_t m; m.menu = n->s; 
#ifdef TEST_MENU_LIST
        m.list.push_back(n->code);
#endif
        n->menus.insert(m);
    }

    // todo: change this to get rid of the perturbed flag
    bool perturbed;
    do {
        perturbed = false;
        for (auto n: menuable_nodes) {
            for (auto m: n->menus) {
                // for each menu of n
                for (auto neighbor: n->neighbors) {
                    if (neighbor->s.empty()) {
                        continue;
                    }

                    // for each neighbor
                    menu_t neigh_new_menu;

#ifdef TEST_MENU_LIST
                    // add the last neighbor to the list of nodes we travelled
                    neigh_new_menu.list = m.list;
                    neigh_new_menu.list.push_back(neighbor->code);
#endif

                    for (auto s_elem: neighbor->s) {
                        // for each s_elem
                        for (auto m_elem: m.menu) {
                            if (m_elem == s_elem ||
                                m_elem->neighbors.find(s_elem) != m_elem->neighbors.end()) {
                                // insert into current menu if we find an appropriate s-neighbor
                                neigh_new_menu.menu.insert(s_elem);
                                break;
                            }
                        }
                    }

                    if (neigh_new_menu.menu.empty()) {
                        std::cout << "Can't project a path" << std::endl;
#ifdef TEST_MENU_LIST
                        for (auto code: neigh_new_menu.list) {
                            display_code(code);
                        }
#endif
                        return;
                    }

                    if (!perturbed) {
                        perturbed = (neighbor->menus.insert(neigh_new_menu)).second;
                    } else {
                        neighbor->menus.insert(neigh_new_menu);
                    }
                }
            }
        }
    } while (perturbed);
}
#endif

#ifdef TEST_BRUTE
void brute_insert_node(node_t* node)
{
    if (cur_index < max_indices && node->index == max_indices) {
        node->index = cur_index;
        node_table[cur_index++] = node;
    } else if (cur_index >= max_indices) {
        not_added++;
    }
}

void test_brute()
{
    for (size_t i = 0; i < cur_index; i++) {
        for (size_t j = i + 1; j < cur_index; j++) {
            // a bit hacky, but alright
            virtual_dist[i][j] = classical_dist[i][j] = 
                virtual_dist[j][i] = classical_dist[j][i] = std::numeric_limits<size_t>::max() / 2;
        }
    }

    for (size_t i = 0; i < cur_index; i++) {
        for (auto iter: node_table[i]->neighbors) {
            if (iter->index < max_indices) {
                virtual_dist[i][iter->index] = virtual_dist[iter->index][i] = 1;
                if (is_planar(node_table[i]) && is_planar(iter)) {
                    classical_dist[i][iter->index] = classical_dist[iter->index][i] = 1;
                }
            }
        }
    }

    for (size_t k = 0; k < cur_index; k++) {
        for (size_t i = 0; i < cur_index; i++) {
            for (size_t j = 0; j <= i; j++) {
                if (virtual_dist[i][j] > virtual_dist[i][k] + virtual_dist[k][j]) {
                    virtual_dist[i][j] = virtual_dist[j][i] =
                                        virtual_dist[i][k] + virtual_dist[k][j];
                }

                if (is_planar(node_table[k]) && is_planar(node_table[i]) && is_planar(node_table[j])) {
                    if (classical_dist[i][j] > classical_dist[i][k] + classical_dist[k][j]) {
                        classical_dist[i][j] = classical_dist[j][i] =
                                        classical_dist[i][k] + classical_dist[k][j];
                    }                    
                }
            }
        }
    }

    for (size_t i = 0; i < cur_index; i++) {
        for (size_t j = i + 1; j < cur_index; j++) {
            if (classical_dist[i][j] != std::numeric_limits<size_t>::max() / 2 &&
                virtual_dist[i][j] < classical_dist[i][j]) {
                std::cout << "Candidate " << stringify_code(node_table[i]->code) << " to " << 
                                             stringify_code(node_table[j]->code);
                std::cout << " with " << virtual_dist[i][j] << " vs " <<  classical_dist[i][j] << std::endl;
            } else if (virtual_dist[i][j] == classical_dist[i][j]) {
                // std::cout << "Equal " << stringify_code(node_table[i]->code) << " to "; display_code(node_table[j]->code);    
            }
        }
    }
}
#endif

void explore()
{
    chunk = new node_t[chunk_size];

    // add the unknot
    // parse_code("U-0U-1O-2O+3U+3U-2U+4O+5O-1O+4U+5O+6U+6O-0")
    node_t* node = prune_ify(code_t());
    explore_complete_neighbors(node);
    for (auto n: node->neighbors) {
        if (n->code.size() > 2 * MAX_CHORDS) continue;

        prune_ify(n);
        explore_special_neighbors(n);
        for (auto nn: n->neighbors) {
            if (nn->code.size() > 2 * MAX_CHORDS) continue;

            prune_ify(nn);
            explore_special_neighbors(nn);
            // explore_complete_neighbors(nn);
            for (auto nnn: nn->neighbors) {
                if (nnn->code.size() > 2 * MAX_CHORDS) continue;

                prune_ify(nnn);
            //     // explore_special_neighbors(nnn);
            //     // // explore_complete_neighbors(nnn);
            //     // for (auto nnnn: nnn->neighbors) {
            //     //     if (n->code.size() > 2 * MAX_CHORDS) continue;

            //     //     prune_ify(nnnn);
            //     // }
            }
        }
    }

#ifdef TEST_MENU
    std::cout << "Number of menuable nodes are " << menuable_nodes.size() << std::endl;
    std::cout << "Propogating menus now" << std::endl;
    test_menu();
    std::cout << "Finished menu test" << std::endl;
#endif

#ifdef TEST_BRUTE
    std::cout << "Added " << cur_index << " nodes" << std::endl;
    std::cout << "Did not add " << not_added << " nodes" << std::endl;
    std::cout << "Beginning brute test" << std::endl;
    test_brute();
    std::cout << "Finished brute test" << std::endl;
#endif

    std::cout << "Beginning hillary test" << std::endl;
    test_hillary();
    std::cout << "Finished hillary test" << std::endl;
}
