#ifndef _VIRTUAL_H
#define _VIRTUAL_H

#include "gauss.h"

// takes in a gauss code and returns true if it's planar/classical
// the cubic version
bool planar_knot_cubic(const code_t& input_code);

// takes in a gauss code and returns true if it's planar/classical
bool planar_knot(const code_t& code);

#endif /* _VIRTUAL_H */
