#ifndef _GRAPH_H
#define _GRAPH_H

#include "gauss.h"
#include <set>
#include <unordered_set>
#include <vector>

#define MAX_CHORDS         13

// #define TEST_MENU
// #define TEST_MENU_LIST
#define TEST_BRUTE

typedef struct menu_t {
    std::set<struct node_t*> menu;

#ifdef TEST_MENU_LIST
    std::vector<code_t> list;
#endif

    bool operator <(const struct menu_t& b) const
    {
        return menu < b.menu;
    }
} menu_t;

typedef struct node_t {
    code_t code;
    int genus;

#ifdef TEST_BRUTE
    size_t index;
#endif

    // subdiagrams
    std::set<struct node_t*> subs;
    // 's' set
    std::set<struct node_t*> s;
    // if we've prunified this
    bool pruneify;

    std::unordered_set<struct node_t*> neighbors;
    bool sneighbors_explored;
    bool neighbors_explored;

    std::set<menu_t> menus;
} node_t;

void explore();

#endif /* _GRAPH_H */
