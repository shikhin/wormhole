#ifndef _GRAPH_H
#define _GRAPH_H

#include "gauss.h"
#include <unordered_set>

#define MAX_CHORDS          12

typedef struct node_t {
    code_t code;
    bool sify;
    bool planar;
    bool neighbors_explored;
    bool pruneify;
    std::unordered_set<struct node_t*> s;
    std::unordered_set<struct node_t*> neighbors;
} node_t;

void explore();

#endif /* _GRAPH_H */
