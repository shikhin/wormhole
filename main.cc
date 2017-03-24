#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include "genus.h"
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

        display_code(code);
        std::cout << "Planar? " << (planar_knot(code) ? "true" : "false") << std::endl;
        std::cout << "Genus: " << genus(code) << std::endl;

        std::cout << "Enumerating r2 moves" << std::endl;
        std::vector<code_t> list = r2_undo_enumerate(code);

        std::sort(list.begin(), list.end());
        list.erase(std::unique(list.begin(), list.end()), list.end());
        for (size_t i = 0; i < list.size(); i++) {
            display_code(list[i]);
        }

        std::vector<code_t> r2_done_list;
        for (size_t i = 0; i < list.size(); i++) {
            std::vector<code_t> app = r2_do_enumerate(list[i]);
            r2_done_list.insert(r2_done_list.end(), app.begin(), app.end());
        }

        std::cout << "Un-enumerating r2 moves" << std::endl;
        std::sort(r2_done_list.begin(), r2_done_list.end());
        r2_done_list.erase(std::unique(r2_done_list.begin(), r2_done_list.end()), r2_done_list.end());
        for (size_t i = 0; i < r2_done_list.size(); i++) {
            display_code(r2_done_list[i]);
        }

        std::cout << "Enumerating r1 moves" << std::endl;
        list = r1_undo_enumerate(code);

        std::sort(list.begin(), list.end());
        list.erase(std::unique(list.begin(), list.end()), list.end());
        for (size_t i = 0; i < list.size(); i++) {
            display_code(list[i]);
        }

        std::vector<code_t> r1_done_list;
        for (size_t i = 0; i < list.size(); i++) {
            std::vector<code_t> app = r1_do_enumerate(list[i]);
            r1_done_list.insert(r1_done_list.end(), app.begin(), app.end());
        }

        std::cout << "Un-enumerating r1 moves" << std::endl;
        std::sort(r1_done_list.begin(), r1_done_list.end());
        r1_done_list.erase(std::unique(r1_done_list.begin(), r1_done_list.end()), r1_done_list.end());
        for (size_t i = 0; i < r1_done_list.size(); i++) {
            display_code(r1_done_list[i]);
        }

        std::cout << "All neighbors" << std::endl;
        list = enumerate_complete_neighbors(code);

        std::sort(list.begin(), list.end());
        list.erase(std::unique(list.begin(), list.end()), list.end());
        for (size_t i = 0; i < list.size(); i++) {
            display_code(list[i]);
        }

        // subdiagrams(code);
    }

    srand(time(NULL));
    subsets_init();

#if 0
    std::vector<std::string> movie;
    movie.push_back("U-0U-1O-2O+3U+3U-2U+4O+5O-1O+4U+5O+6U+6O-0");
    movie.push_back("U-0U-1U-2U+3O-4O+5U+5U-4U+6O+7O-1O+6O+3O-2U+7O+8U+8O-0");
    movie.push_back("U-0U-1U-2U+3U+4O+5O-1O+4O+3O-2U+5O+6U+6O-0");
    movie.push_back("U-0U-1U-2O-3O+4U+4U-3U+5U+6O+7O-1O+6O+5O-2U+7O+8U+8O-0");
    movie.push_back("U-0U-1U-2O-3O+4U+4U-3U+5U+6O+7O-1O+6O+5O-2U+7O-0");
    movie.push_back("U-0U-1U-2O-3O+4U+4U-3U+5U+6O-0O+7O+6O+5O-2O-1U+7");
    movie.push_back("U-0O-0O+1O-2O-3U+4U-5U-3U-2O-6O+7U+7U-6U+8U+9O-5O+4O+9O+8U+1");
    movie.push_back("U-0O-0O+1O-2O-3U-3U-2O-4O+5U+5U-4U+6U+7O+7O+6U+1");

    cleanup_movie(movie);
#endif

    explore();

    return 0;
}
