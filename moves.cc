#include "moves.h"
#include <set>
#include <vector>

// cleans up a movie so the unsanitary versions are the ones that are displayed
// yes, some people like unsanitary movies
void cleanup_movie(std::vector<std::string> movie)
{
    std::vector<code_t> list;
    for (auto &str: movie) {
        list.push_back(parse_code(str));
    }

    auto prev = list[0];
    for (auto cur: list) {
        auto neighbors = enumerate_complete_unsan_neighbors(prev);
        code_t next = code_t();
        for (auto iter: neighbors) {
            auto san = iter;

            size_t max = 0;
            for (auto code_iter: san) {
                max = std::max(max, (size_t) ELEM_ID(code_iter));
            }
            renumber_code(san, max + 1);
            san = first_ordered_code(san);

            if (!compare_codes(san, cur)) {
                prev = iter;
                break;
            }
        }
        display_code(prev);
    }
}

// inserts a R1 move before x
// positive         - if the (first, for flat knots) sign is positive
// first_over       - if first element is over
#ifdef FLAT_KNOTS
static code_t r1_undo_unsan(code_t code, size_t x, bool positive)
#else
static code_t r1_undo_unsan(code_t code, size_t x, bool positive, bool first_over)
#endif
{
    code_elem_t first;

    ssize_t max = -1;
    for (auto iter: code) {
        max = std::max(max, (ssize_t) ELEM_ID(iter));
    }

    // first = (code.size() / 2) << ELEM_ID_SHIFT;
    first = (max + 1) << ELEM_ID_SHIFT;
    if (positive) {
        first |= ELEM_POSITIVE;
    }

#ifndef FLAT_KNOTS
    if (first_over) {
        first |= ELEM_OVER;
    }
#endif

    // the second is the same as first, just with OVER/SIGN bit reversed
    code_t ins; ins.push_back(first);
#ifndef FLAT_KNOTS
    ins.push_back(first ^ ELEM_OVER);
#else
    ins.push_back(first ^ ELEM_POSITIVE);
#endif
    code.insert(code.begin() + x, ins.begin(), ins.end());

    return code;
}

// inserts a R1 move before x
// positive         - if the sign is positive
// first_over       - if first element is over
#ifdef FLAT_KNOTS
static code_t r1_undo(code_t code, size_t x, bool positive)
#else
static code_t r1_undo(code_t code, size_t x, bool positive, bool first_over)
#endif
{
#ifdef FLAT_KNOTS
    code = r1_undo_unsan(code, x, positive);
#else
    code = r1_undo_unsan(code, x, positive, first_over);
#endif

    // standardize it
    renumber_code(code, code.size() / 2);
    return first_ordered_code(code);
}

#ifdef FLAT_KNOTS
std::vector<code_t> r1_undo_raw_enumerate(const code_t& code, code_t (*r1_undo)(code_t, size_t, bool))
#else
std::vector<code_t> r1_undo_raw_enumerate(const code_t& code, code_t (*r1_undo)(code_t, size_t, bool, bool))
#endif
{
    std::vector<code_t> list; size_t length = code.size();
    size_t x = 0;
    do {
#ifdef FLAT_KNOTS
        list.push_back(r1_undo(code, x, false));
        list.push_back(r1_undo(code, x, true));   
#else
        // 4 possible choices, so just go over all of them
        for (int i = 0; i < 4; i++) {
            list.push_back(r1_undo(code, x, (i >> 0) & 1, (i >> 1) & 1));
        }
#endif
        x++;
    } while (x < length);

    return list;
}

std::vector<code_t> r1_undo_enumerate(const code_t& code)
{
    return r1_undo_raw_enumerate(code, r1_undo);
}

std::vector<code_t> r1_undo_unsan_enumerate(const code_t& code)
{
    return r1_undo_raw_enumerate(code, r1_undo_unsan);
}

// do a R1 move on x
static code_t r1_do_unsan(const code_t& code, size_t x)
{
    code_t moved; size_t length = code.size();
    for (size_t i = 0; i < length; i++) {
        if ((i == x) || (i == ((x + 1) % length))) {
            // if these are it
            continue;
        }

        moved.push_back(code[i]);
    }

    return moved;
}

// do a R1 move on x
static code_t r1_do(const code_t& code, size_t x)
{
    code_t moved = r1_do_unsan(code, x);

    renumber_code(moved, code.size() / 2);
    return first_ordered_code(moved);
}

std::vector<code_t> r1_do_raw_enumerate(const code_t& code, code_t (*r1_do)(const code_t&, size_t))
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

std::vector<code_t> r1_do_enumerate(const code_t& code)
{
    return r1_do_raw_enumerate(code, r1_do);
}

std::vector<code_t> r1_do_unsan_enumerate(const code_t& code)
{
    return r1_do_raw_enumerate(code, r1_do_unsan);
}

// inserts a R2 move before y and x, in that order
// first_positive   - if first ID is positive
// first_over       - if first pair is over
// flip             - if the order is flipped in second part
#ifdef FLAT_KNOTS
static code_t r2_undo_unsan(code_t code, size_t x, size_t y,
               bool first_positive, bool flip)
#else
static code_t r2_undo_unsan(code_t code, size_t x, size_t y,
               bool first_positive, bool first_over, bool flip)
#endif
{
    code_elem_t first, second, third, fourth;

    ssize_t max = -1;
    for (auto iter: code) {
        max = std::max(max, (ssize_t) ELEM_ID(iter));
    }

    first = (max + 1) << ELEM_ID_SHIFT; second = (max + 2) << ELEM_ID_SHIFT;
    // first = (code.size() / 2) << ELEM_ID_SHIFT; second = (code.size() / 2 + 1) << ELEM_ID_SHIFT;
    if (first_positive) {
        first |= ELEM_POSITIVE;
    } else {
        second |= ELEM_POSITIVE;
    }

#ifndef FLAT_KNOTS
    if (first_over) {
        first |= ELEM_OVER;
        second |= ELEM_OVER;
    }
#endif

#ifdef FLAT_KNOTS
    if (flip) {
        third = second ^ ELEM_POSITIVE;
        fourth = first ^ ELEM_POSITIVE;
    } else {
        third = first ^ ELEM_POSITIVE;
        fourth = second ^ ELEM_POSITIVE;
    }
#else
    if (flip) {
        third = second ^ ELEM_OVER;
        fourth = first ^ ELEM_OVER;
    } else {
        third = first ^ ELEM_OVER;
        fourth = second ^ ELEM_OVER;
    }
#endif

    code_t ins_1; ins_1.push_back(third); ins_1.push_back(fourth);
    code.insert(code.begin() + y, ins_1.begin(), ins_1.end());
    code_t ins_2; ins_2.push_back(first); ins_2.push_back(second);
    code.insert(code.begin() + x, ins_2.begin(), ins_2.end());

    return code;
}

// inserts a R2 move before y and x, in that order
// first_positive   - if first ID is positive
// first_over       - if first pair is over
// flip             - if the order is flipped in second part
#ifdef FLAT_KNOTS
static code_t r2_undo(code_t code, size_t x, size_t y,
               bool first_positive, bool flip)
#else
static code_t r2_undo(code_t code, size_t x, size_t y,
               bool first_positive, bool first_over, bool flip)
#endif
{
#ifdef FLAT_KNOTS
    code = r2_undo_unsan(code, x, y, first_positive, flip);
#else
    code = r2_undo_unsan(code, x, y, first_positive, first_over, flip);
#endif
    renumber_code(code, code.size() / 2);
    return first_ordered_code(code);
}

#ifdef FLAT_KNOTS
std::vector<code_t> r2_undo_raw_enumerate(const code_t& code,
                                            code_t (*r2_undo)(code_t, size_t, size_t, bool, bool))
#else
std::vector<code_t> r2_undo_raw_enumerate(const code_t& code,
                                            code_t (*r2_undo)(code_t, size_t, size_t, bool, bool, bool))
#endif
{
    std::vector<code_t> list; size_t length = code.size();
    size_t x = 0, y = 0;
    do {
        y = x;
        do {
#ifdef FLAT_KNOTS
            // 4 possible choices, so just go over all of them
            for (int i = 0; i < 4; i++) {
                list.push_back(r2_undo(code, x, y, (i >> 0) & 1, (i >> 1) & 1));
            }
#else
            // 8 possible choices, so just go over all of them
            for (int i = 0; i < 8; i++) {
                list.push_back(r2_undo(code, x, y, (i >> 0) & 1, (i >> 1) & 1, (i >> 2) & 1));
            }
#endif

            y++;
        } while (y < length);
        x++;
    } while (x < length);

    return list;
}


std::vector<code_t> r2_undo_enumerate(const code_t& code)
{
    return r2_undo_raw_enumerate(code, r2_undo);
}

std::vector<code_t> r2_undo_unsan_enumerate(const code_t& code)
{
    return r2_undo_raw_enumerate(code, r2_undo_unsan);
}

// do a R2 move on x and y
static code_t r2_do_unsan(const code_t& code, size_t x, size_t y)
{
    code_t moved; size_t length = code.size();
    for (size_t i = 0; i < length; i++) {
        if ((i == x) || (i == ((x + 1) % length)) ||
            (i == y) || (i == ((y + 1) % length))) {
            // if these are it
            continue;
        }

        moved.push_back(code[i]);
    }

    return moved;
}

// do a R2 move on x and y
static code_t r2_do(const code_t& code, size_t x, size_t y)
{
    code_t moved = r2_do_unsan(code, x, y);

    renumber_code(moved, code.size() / 2);
    return first_ordered_code(moved);
}

std::vector<code_t> r2_do_raw_enumerate(const code_t& code, code_t (*r2_do)(const code_t&, size_t, size_t))
{
    std::vector<code_t> list; int length = code.size();

    for (int x = 0; x < length - 2; x++) {
        size_t x_ = x + 1;
        // check if x and x_ meet the requirements
        code_elem_t id_x = code[x], id_x_ = code[x_];
        if ((id_x & ELEM_SIGN_MASK) == (id_x_ & ELEM_SIGN_MASK)) {
            continue;
        }

#ifndef FLAT_KNOTS
        if ((id_x & ELEM_OU_MASK) != (id_x_ & ELEM_OU_MASK)) {
            continue;
        }
#endif

        id_x = ELEM_ID(id_x); id_x_ = ELEM_ID(id_x_);

        for (size_t y = x + 2; y < length; y++) {
            size_t y_ = (y + 1) % length;
            // can't overlap
            if (y_ == x) continue;

            if ((id_x == ELEM_ID(code[y_]) && id_x_ == ELEM_ID(code[y])) ||
                (id_x == ELEM_ID(code[y]) && id_x_ == ELEM_ID(code[y_]))) {
                list.push_back(r2_do(code, x, y));
                continue;
            }
        }
    }

    return list;
}

std::vector<code_t> r2_do_enumerate(const code_t& code)
{
    return r2_do_raw_enumerate(code, r2_do);
}

std::vector<code_t> r2_do_unsan_enumerate(const code_t& code)
{
    return r2_do_raw_enumerate(code, r2_do_unsan);
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
static code_t r3_unsan(const code_t& code, size_t x, size_t y, size_t z)
{
    return r3_vanilla(code, x, y, z);
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

#ifdef FLAT_KNOTS
    if (SIGN(code[x]) == SIGN(code[y]) && SIGN(code[y]) == SIGN(code[z]) &&
        SIGN(code[x_]) == SIGN(code[y_]) && SIGN(code[y_]) == SIGN(code[z_]) &&
        SIGN(code[x]) != SIGN(code[x_])) {
        return true;
    } else {
        return false;
    }
#else
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
#endif
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

#ifdef FLAT_KNOTS
    if (SIGN(code[x]) == SIGN(code[x_]) && SIGN(code[y]) == SIGN(code[y_]) &&
        SIGN(code[z]) == SIGN(code[y]) && SIGN(code[z_]) == SIGN(code[x]) &&
        SIGN(code[x]) != SIGN(code[y])) {
        return true;
    } else {
        return false;
    }
#else
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
#endif
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

std::vector<code_t> r3_raw_enumerate(const code_t& code, code_t (*r3)(const code_t&, size_t, size_t, size_t))
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

std::vector<code_t> r3_enumerate(const code_t& code)
{
    return r3_raw_enumerate(code, r3);
}

std::vector<code_t> r3_unsan_enumerate(const code_t& code)
{
    return r3_raw_enumerate(code, r3_unsan);
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

// enumerate neighbors of code
std::vector<code_t> enumerate_complete_unsan_neighbors(const code_t& code)
{
    std::vector<code_t> list, t;

    // r1
    list = r1_do_unsan_enumerate(code);
    t = r1_undo_unsan_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());

    // r2
    t = r2_do_unsan_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());
    t = r2_undo_unsan_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());

    // r3
    t = r3_unsan_enumerate(code);
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
std::vector<code_t> enumerate_nonspecial_neighbors(const code_t& code)
{
    std::vector<code_t> list, t;

    // r1
    list = r1_undo_enumerate(code);

    // r2
    t = r2_undo_enumerate(code);
    list.insert(list.end(), t.begin(), t.end());

    return list;
}
