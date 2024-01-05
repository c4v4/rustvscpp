#include <fmt/chrono.h>
#include <fmt/core.h>

#include <chrono>
#include <numeric>

#include "Instance.hpp"

void print_tour(Instance &inst, std::vector<int> &tour);
std::vector<int> nn_tour(Instance &inst);
void two_opt_full(Instance &inst, std::vector<int> &tour);
int i_loop(const Instance &inst, std::vector<int> &tour, int n, int i, std::vector<char> &skipbits);
int two_opt_gain(const Instance &inst, std::vector<int> &tour, int i, int j);
void two_opt_apply(std::vector<int> &tour, int i, int j);

int main(int argc, char **args) {
    auto start = std::chrono::steady_clock::now();
    auto t1 = std::chrono::steady_clock::now();
    if (argc < 1) {
        fmt::print("Needs instance path.\n");
        abort();
    }

    Instance inst(args[1]);
    fmt::print("Name: {}, Dist: {}, Size: {}\n", inst.get_name(), inst.get_dist_type(), inst.size());
    auto t2 = std::chrono::steady_clock::now();
    fmt::print("Parsing instance time: {} ms.\n", std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1));

    fmt::print("NN tour ");
    auto tour = nn_tour(inst);
    print_tour(inst, tour);
    t1 = std::chrono::steady_clock::now();
    fmt::print("Nearest neighbor greedy time: {} ms.\n", std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t2));

    fmt::print("Applying 2Opt");
    two_opt_full(inst, tour);
    t2 = std::chrono::steady_clock::now();
    fmt::print("Full 2-Opt time: {} ms.\n", std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1));

    fmt::print("Should exit after one full loop2\n");
    two_opt_full(inst, tour);
    print_tour(inst, tour);
    t1 = std::chrono::steady_clock::now();
    fmt::print("Check 2-Opt time: {} ms.\n", std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t2));
    fmt::print("Total time: {} ms.\n", std::chrono::duration_cast<std::chrono::milliseconds>(t1 - start));

    return EXIT_SUCCESS;
}

void print_tour(Instance &inst, std::vector<int> &tour) {
    int length = 0;
    for (int v = 1; v < inst.size(); ++v) {
        int l = inst.dist(tour[v - 1], tour[v]);
        length += l;
    }
    length += inst.dist(tour[inst.size() - 1], tour[0]);
    fmt::print("Lengh: {}; \n", length);
}

auto nn_tour(Instance &inst) -> std::vector<int> {
    int n = inst.size();
    auto tour = std::vector<int>(n);
    std::iota(tour.begin(), tour.end(), 0);

    for (int i = 0; i < n - 2; ++i) {
        int nearest = i + 1;
        int d_nearest = inst.dist(tour[i], tour[i + 1]);
        for (int j = i + 2; j < n; ++j) {
            int const d_candidate = inst.dist(tour[i], tour[j]);
            if (d_candidate < d_nearest) {
                nearest = j;
                d_nearest = d_candidate;
            }
        }
        std::swap(tour[i + 1], tour[nearest]);
    }
    return tour;
}

void two_opt_full(Instance &inst, std::vector<int> &tour) {

    int const n = inst.size();
    int ext_pass_gain = 0;
    std::vector<char> skipbits(n);
    do {
        fmt::print("loop 1\n");
        ext_pass_gain = 0;
        int pass_gain = 0;
        do {
            fmt::print("  loop 2\n");
            pass_gain = 0;
            for (int i = 0; i < n - 1; ++i)
                pass_gain += i_loop(inst, tour, n, i, skipbits);
            ext_pass_gain += pass_gain;
        } while (pass_gain > 0);
    } while (ext_pass_gain > 0);
}

int i_loop(const Instance &inst, std::vector<int> &tour, int n, int i, std::vector<char> &skipbits) {

    auto i_gain = 0;
    for (int j = i + 2; j < n; ++j) {
        auto const gain = two_opt_gain(inst, tour, i, j);
        if (gain > 0) {
            i_gain += gain;
            two_opt_apply(tour, i, j);
            skipbits[j] = false;
        }
    }

    if (i_gain == 0)
        skipbits[i] = true;

    return i_gain;
}

int two_opt_gain(const Instance &inst, std::vector<int> &tour, int i, int j) {
    auto const n = inst.size();
    auto const jnext = j != n - 1 ? j + 1 : 0;
    auto const added = inst.dist(tour[i], tour[j]) + inst.dist(tour[i + 1], tour[jnext]);
    auto const removed = inst.dist(tour[i], tour[i + 1]) + inst.dist(tour[j], tour[jnext]);
    return removed - added;
}

void two_opt_apply(std::vector<int> &tour, int i, int j) { std::reverse(tour.begin() + i + 1, tour.begin() + j + 1); }
