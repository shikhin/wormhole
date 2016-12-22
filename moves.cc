#include "moves.h"
#include <set>
#include <vector>

// inserts a R1 move before x
// positive         - if the sign is positive
// first_over       - if first element is over
static code_t r1_undo(code_t code, size_t x, bool positive, bool first_over)
{
    code_elem_t first, second;
    first = (code.size() / 2) << ELEM_ID_SHIFT;
    if (positive) {
        first |= ELEM_POSITIVE;
    }

    if (first_over) {
        first |= ELEM_OVER;
    }

    // the second is the same as first, just with OVER bit reversed
    second = first ^ ELEM_OVER;

    code_t ins; ins.push_back(first); ins.push_back(second);
    code.insert(code.begin() + x, ins.begin(), ins.end());

    // standardize it
    renumber_code(code, code.size() / 2);
    return first_ordered_code(code);
}

std::vector<code_t> r1_undo_enumerate(const code_t& code)
{
    std::vector<code_t> list; size_t length = code.size();
    size_t x = 0;
    do {
        // 4 possible choices, so just go over all of them
        for (int i = 0; i < 4; i++) {
            list.push_back(r1_undo(code, x, (i >> 0) & 1, (i >> 1) & 1));
        }

        x++;
    } while (x < length);

    return list;
}

// do a R1 move on x
static code_t r1_do(const code_t& code, size_t x)
{
    // todo: could renumber and remove in one go if it
    // matters
    code_t moved; size_t length = code.size();
    for (size_t i = 0; i < length; i++) {
        if ((i == x) || (i == ((x + 1) % length))) {
            // if these are it
            continue;
        }

        moved.push_back(code[i]);
    }

    renumber_code(moved, code.size() / 2);
    return first_ordered_code(moved);
}

std::vector<code_t> r1_do_enumerate(const code_t& code)
{
    std::vector<code_t> list; size_t length = code.size();
    if (length < 2) {
        return list;
    }

    for (size_t x = 0; x < length; x++) {
        size_t x_ = (x + 1) % length;
        // check if x and x_ meet the requirements
        if (ELEM_ID(code[x]) == ELEM_ID(code[x_])) {
            list.push_back(r1_do(code, x));
        }
    }

    return list;
}

// inserts a R2 move before y and x, in that order
// first_positive   - if first ID is positive
// first_over       - if first pair is over
// flip             - if the order is flipped in second part
static code_t r2_undo(code_t code, size_t x, size_t y,
               bool first_positive, bool first_over, bool flip)
{
    code_elem_t first, second, third, fourth;
    first = (code.size() / 2) << ELEM_ID_SHIFT; second = (code.size() / 2 + 1) << ELEM_ID_SHIFT;
    if (first_positive) {
        first |= ELEM_POSITIVE;
    } else {
        second |= ELEM_POSITIVE;
    }

    if (first_over) {
        first |= ELEM_OVER;
        second |= ELEM_OVER;
    }

    if (flip) {
        third = second ^ ELEM_OVER;
        fourth = first ^ ELEM_OVER;
    } else {
        third = first ^ ELEM_OVER;
        fourth = second ^ ELEM_OVER;
    }

    code_t ins_1; ins_1.push_back(third); ins_1.push_back(fourth);
    code.insert(code.begin() + y, ins_1.begin(), ins_1.end());
    code_t ins_2; ins_2.push_back(first); ins_2.push_back(second);
    code.insert(code.begin() + x, ins_2.begin(), ins_2.end());

    // standardize it
    renumber_code(code, code.size() / 2);
    return first_ordered_code(code);
}

std::vector<code_t> r2_undo_enumerate(const code_t& code)
{
    std::vector<code_t> list; size_t length = code.size();
    size_t x = 0, y = 0;
    do {
        y = x;
        do {
            // 8 possible choices, so just go over all of them
            for (int i = 0; i < 8; i++) {
                list.push_back(r2_undo(code, x, y, (i >> 0) & 1, (i >> 1) & 1, (i >> 2) & 1));
            }
            y++;
        } while (y < length);
        x++;
    } while (x < length);

    return list;
}

// do a R2 move on x and y
static code_t r2_do(const code_t& code, size_t x, size_t y)
{
    // todo: could renumber and remove in one go if it
    // matters
    code_t moved; size_t length = code.size();
    for (size_t i = 0; i < length; i++) {
        if ((i == x) || (i == ((x + 1) % length)) ||
            (i == y) || (i == ((y + 1) % length))) {
            // if these are it
            continue;
        }

        moved.push_back(code[i]);
    }

    renumber_code(moved, code.size() / 2);
    return first_ordered_code(moved);
}

std::vector<code_t> r2_do_enumerate(const code_t& code)
{
    std::vector<code_t> list; int length = code.size();

    for (int x = 0; x < length - 2; x++) {
        size_t x_ = x + 1;
        // check if x and x_ meet the requirements
        code_elem_t id_x = code[x], id_x_ = code[x_];
        if ((id_x & ELEM_SIGN_MASK) == (id_x_ & ELEM_SIGN_MASK) ||
            (id_x & ELEM_OU_MASK) != (id_x_ & ELEM_OU_MASK)) {
            continue;
        }

        id_x >>= ELEM_ID_SHIFT; id_x_ >>= ELEM_ID_SHIFT;

        for (size_t y = x + 2; y < length; y++) {
            size_t y_ = (y + 1) % length;
            // can't overlap
            if (y_ == x) continue;

            if ((id_x == (code[y_] >> ELEM_ID_SHIFT) && id_x_ == (code[y] >> ELEM_ID_SHIFT)) ||
                (id_x == (code[y] >> ELEM_ID_SHIFT) && id_x_ == (code[y_] >> ELEM_ID_SHIFT))) {
                list.push_back(r2_do(code, x, y));
                continue;
            }
        }
    }

    return list;
}

// a R3 move at x, y, z presuming it's valid
// don't renumber or reorder
static code_t r3_vanilla(code_t code, size_t x, size_t y, size_t z)
{
    size_t length = code.size();
    code_elem_t temp;

    temp = code[x];
    code[x] = code[(x + 1) % length];
    code[(x + 1) % length] = temp;

    temp = code[y];
    code[y] = code[(y + 1) % length];
    code[(y + 1) % length] = temp;

    temp = code[z];
    code[z] = code[(z + 1) % length];
    code[(z + 1) % length] = temp;

    return code;
}

// a R3 move, and renumber and reorder
static code_t r3(const code_t& code, size_t x, size_t y, size_t z)
{
    code_t moved = r3_vanilla(code, x, y, z);
    renumber_code(moved, moved.size() / 2);
    return first_ordered_code(moved);
}

static bool is_triangular(const code_t& code, size_t x, size_t x_, 
                                              size_t y, size_t y_,
                                              size_t z, size_t z_)
{
    size_t length = code.size();
    code_elem_t first = ELEM_ID(code[x]),
                second = ELEM_ID(code[x_]),
                third = ELEM_ID(code[y_]);
    if (ELEM_ID(code[z_]) != first ||
        ELEM_ID(code[y]) != second ||
        ELEM_ID(code[z]) != third) {
        return false;
        // checked triangular nature
    }

    int sum = SIGN(code[x]) + SIGN(code[x_]) + SIGN(code[y_]);
    if (sum != -1) {
        return false;
        // two negatives and one positive
    }

    if (!OVER(code[x]) || !OVER(code[x_])) {
        return false;
        // both at first segment should be over
    }

    if (OVER(code[y_])) {
        if (SIGN(code[x]) != +1) {
            return false;
        }
    } else if (SIGN(code[x_]) != +1) {
        return false;
    }
    // the correct one should be +1

    return true;
}

// the case with the single crossing (single crossing, triangular,
// crossingular...)
static bool is_crossingular(const code_t& code, size_t x, size_t x_,
                                                size_t y, size_t y_,
                                                size_t z, size_t z_)
{
    size_t length = code.size();
    code_elem_t first = ELEM_ID(code[x]),
                second = ELEM_ID(code[x_]),
                third = ELEM_ID(code[y_]);
    if (ELEM_ID(code[z]) != first ||
        ELEM_ID(code[y]) != second ||
        ELEM_ID(code[z_]) != third) {
        return false;
        // checked crossingular nature
    }

    int sum = SIGN(code[x]) + SIGN(code[x_]) + SIGN(code[y_]);
    if (sum == -1) {
        // two negatives and one positive
        if (SIGN(code[x_]) != -1) {
            return false;
        }

        // check the over/under pattern is fine
        if (OVER(code[x]) != OVER(code[y_])) {
            return false;
        }

        // some stupid cases
        // can collapse a bit, but easier to read this way
        if (OVER(code[x_])) {
            if (OVER(code[x])) {
                if (SIGN(code[x]) != -1) {
                    return false;
                }
            } else if (SIGN(code[x]) != +1) {
                return false;
            }
        } else {
            if (OVER(code[x])) {
                if (SIGN(code[x]) != +1) {
                    return false;
                }
            } else if (SIGN(code[x]) != -1) {
                return false;
            }
        }

        return true;
    } else if (sum == +3) {
        // all positives
        if (OVER(code[x_])) {
            if (!OVER(code[x]) || OVER(code[y_])) {
                return false;
            }
        } else if (OVER(code[x]) || !OVER(code[y_])) {
            return false;
        }

        return true;
    } else {
        return false;
    }
}

// check if we can do a r3 move at x, y, z
static bool can_r3(const code_t& code, size_t x, size_t y, size_t z)
{
    // note: horrible, HORRIBLE, code, but I could not think of a better way to do this

    size_t x_ = (x + 1) % code.size(), y_ = (y + 1) % code.size(), z_ = (z + 1) % code.size();

    // see if they match these without swapping
    if (is_triangular(code, x, x_, y, y_, z, z_) || is_crossingular(code, x, x_, y, y_, z, z_) ||
        is_triangular(code, x_, x, y_, y, z_, z) || is_crossingular(code, x_, x, y_, y, z_, z)) {
        return true;
    }

    return false;
}

std::vector<code_t> r3_enumerate(const code_t& code)
{
    std::vector<code_t> list; size_t length = code.size();
    if (length < 6) {
        // need at least 3 chords
        return list;
    }

    for (size_t x = 0; x < length; x++) {
        for (size_t y = (x + 2) % length; (y + 3) % length != x; y = (y + 1) % length) {
            for (size_t z = (y + 2) % length; (z + 1) % length != x; z = (z + 1) % length) {
                if (can_r3(code, x, y, z)) {
                    list.push_back(r3(code, x, y, z));
                }
            }
        }
    }

    return list;
}

// enumerate neighbors of code
std::vector<code_t> enumerate_complete_neighbors(const code_t& code)
{
    std::vector<code_t> list, t;

    // r1
    list = r1_do_enumerate(code);
    t = r1_undo_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());

    // r2
    t = r2_do_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());
    t = r2_undo_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());

    // r3
    t = r3_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());

    return list;
}

// enumerate neighbors of code such that if enumerated on X and Y, they'll
// be connected if they can be
std::vector<code_t> enumerate_special_neighbors(const code_t& code)
{
    std::vector<code_t> list, t;

    // r1
    list = r1_do_enumerate(code);

    // r2
    t = r2_do_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());

    // r3
    t = r3_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());

    return list;
}

// enumerates neighbors not enumerated by rest
std::vector<code_t> enumerate_rest_neighbors(const code_t& code)
{
    std::vector<code_t> list, t;

    // r1
    list = r1_undo_enumerate(code);

    // r2
    t = r2_undo_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());

    return list;
}
