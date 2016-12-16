#include <algorithm>
#include <cstdlib>
#include <ctime>
#include "graph.h"
#include "gauss.h"
#include <iostream>
#include "moves.h"
#include "subdiag.h"
#include <vector>
#include "virtual.h"

int main(int argc, char const *argv[])
{
    if (argc > 1) {
        code_t code = parse_code(std::string(argv[1]));
        renumber_code(code, code.size() / 2 + 1);
        code = first_ordered_code(code);

        display_code(code);
        std::cout << "Planar? " << (planar_knot(code) ? "true" : "false") << std::endl;
        std::cout << "Enumerating r2 moves" << std::endl;
        std::vector<code_t> list = r2_undo_enumerate(code);

        std::sort(list.begin(), list.end(), comparator_codes);
        list.erase(std::unique(list.begin(), list.end(), equal_codes), list.end());
        for (size_t i = 0; i < list.size(); i++) {
            display_code(list[i]);
        }

        std::vector<code_t> r2_done_list;
        for (size_t i = 0; i < list.size(); i++) {
            std::vector<code_t> app = r2_do_enumerate(list[i]);
            r2_done_list.insert(r2_done_list.end(), app.begin(), app.end());
        }

        std::cout << "Un-enumerating r2 moves" << std::endl;
        std::sort(r2_done_list.begin(), r2_done_list.end(), comparator_codes);
        r2_done_list.erase(std::unique(r2_done_list.begin(), r2_done_list.end(), equal_codes), r2_done_list.end());
        for (size_t i = 0; i < r2_done_list.size(); i++) {
            display_code(r2_done_list[i]);
        }

        std::cout << "Enumerating r1 moves" << std::endl;
        list = r1_undo_enumerate(code);

        std::sort(list.begin(), list.end(), comparator_codes);
        list.erase(std::unique(list.begin(), list.end(), equal_codes), list.end());
        for (size_t i = 0; i < list.size(); i++) {
            display_code(list[i]);
        }

        std::vector<code_t> r1_done_list;
        for (size_t i = 0; i < list.size(); i++) {
            std::vector<code_t> app = r1_do_enumerate(list[i]);
            r1_done_list.insert(r1_done_list.end(), app.begin(), app.end());
        }

        std::cout << "Un-enumerating r1 moves" << std::endl;
        std::sort(r1_done_list.begin(), r1_done_list.end(), comparator_codes);
        r1_done_list.erase(std::unique(r1_done_list.begin(), r1_done_list.end(), equal_codes), r1_done_list.end());
        for (size_t i = 0; i < r1_done_list.size(); i++) {
            display_code(r1_done_list[i]);
        }

        std::cout << "All neighbors" << std::endl;
        list = enumerate_neighbors(code);

        std::sort(list.begin(), list.end(), comparator_codes);
        list.erase(std::unique(list.begin(), list.end(), equal_codes), list.end());
        for (size_t i = 0; i < list.size(); i++) {
            display_code(list[i]);
        }

        // subdiagrams(code);
    }

    srand(time(NULL));
    subsets_init();

    explore();

    return 0;
}
