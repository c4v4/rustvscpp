#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <chrono>
#include <random>

#include "Instance.hpp"

#if defined(__clang__)
#define ASSUME(X) __builtin_assume(X)

#elif defined(__GNUC__) || defined(__GNUG__)
#define ASSUME(X)                \
    while (!(X)) {               \
        __builtin_unreachable(); \
    }

#elif defined(_MSC_VER)
#define ASSUME(X) __assume(X)
#endif

struct node {
    int pred, pred_cost, succ, succ_cost;
};

void link(const Instance &inst, std::vector<node> &tour, int v, int u) {
    tour[v].succ = u;
    tour[u].pred = v;
    tour[v].succ_cost = tour[u].pred_cost = inst.dist(v, u);
}

void print_tour(Instance &inst, std::vector<node> &tour) {
    int length = 0;
    for (auto n : tour) {
        length += n.succ_cost;
    }
    fmt::print("Lengh: {}; \n", length);
}

std::vector<node> nn_tour(Instance &inst) {
    int n = inst.size();
    std::vector<node> tour(n);
    for (int v = 1; v < n; ++v) {
        link(inst, tour, v - 1, v);
    }
    link(inst, tour, n - 1, 0);

    int v1 = 0;
    for (int i = 0; i < n - 2; ++i) {
        auto nearest = -1;
        auto d_nearest = INT32_MAX;
        for (int v2 = tour[v1].succ; v2 != v1; v2 = tour[v2].succ) {
            auto const d_candidate = inst.dist(v1, v2);
            if (d_candidate < d_nearest) {
                nearest = v2;
                d_nearest = d_candidate;
            }
        }
        int v1_succ = tour[v1].succ;
        int v1_succ_succ = tour[v1_succ].succ;
        int nearest_pred = tour[nearest].pred;
        int nearest_succ = tour[nearest].succ;

        link(inst, tour, v1, nearest);
        link(inst, tour, nearest, v1_succ_succ);
        link(inst, tour, nearest_pred, v1_succ);
        link(inst, tour, v1_succ, nearest_succ);
        v1 = nearest;
    }
    return tour;
}

void two_opt_apply(const Instance &inst, std::vector<node> &tour, int i, int j) {
    int i_succ = tour[i].succ;
    int j_succ = tour[j].succ;
    while (j != j_succ) {
        std::swap(tour[j].succ, tour[j].pred);
        std::swap(tour[j].succ_cost, tour[j].pred_cost);
        j = tour[i].succ;
    }
    link(inst, tour, i, j);
    link(inst, tour, i_succ, j_succ);
}

int two_opt_gain(const Instance &inst, std::vector<node> &tour, int i, int j) {
    auto const n = inst.size();
    auto const added = inst.dist(i, j) + inst.dist(tour[i].succ, tour[j].succ);
    auto const removed = tour[i].succ_cost + tour[j].succ_cost;
    return removed - added;
}

int i_loop(const Instance &inst, std::vector<node> &tour, int n, int i, std::vector<int> &next_vcache) {
    auto i_gain = 0;
    for (int j = i + 2; j < n; ++j) {
        auto const gain = two_opt_gain(inst, tour, i, j);
        if (gain > 0) {
            i_gain += gain;
            two_opt_apply(inst, tour, i, j);
            next_vcache.emplace_back(j);
        }
    }

    if (i_gain > 0)
        next_vcache.emplace_back(i);

    return i_gain;
}

void two_opt_full(Instance &inst, std::vector<node> &tour) {
    int const n = inst.size();
    int ext_pass_gain = 0;
    std::vector<int> vcache, next_vcache;
    do {
        fmt::print("loop 1\n");
        ext_pass_gain = 0;
        int pass_gain = 0;
        bool do_all = true;
        do {
            fmt::print("  loop 2\n");
            pass_gain = 0;
            if (do_all) {
                do_all = false;
                for (int i = 0; i < (n - 1); ++i)
                    pass_gain += i_loop(inst, tour, n, i, next_vcache);
            } else {
                while (!vcache.empty()) {
                    int i = vcache.back();
                    vcache.pop_back();
                    pass_gain += i_loop(inst, tour, n, i, next_vcache);
                }
                std::swap(vcache, next_vcache);
            }
            ext_pass_gain += pass_gain;
        } while (pass_gain > 0);
    } while (ext_pass_gain > 0);
}

int main(int argc, char **args) {
    if (argc < 1) {
        fmt::print("Needs instance path.\n");
        abort();
    }

    Instance inst(args[1]);

    fmt::print("Name: {}, Dist: {}, Size: {}\n", inst.get_name(), inst.get_dist_type(), inst.size());

    fmt::print("NN tour ");

    auto start = std::chrono::steady_clock::now();
    auto tour = nn_tour(inst);
    auto end = std::chrono::steady_clock::now();
    fmt::print("({})\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    print_tour(inst, tour);

    fmt::print("Applying 2Opt");

    start = std::chrono::steady_clock::now();
    two_opt_full(inst, tour);
    end = std::chrono::steady_clock::now();
    fmt::print("({})\n", std::chrono::duration_cast<std::chrono::milliseconds>(end - start));

    fmt::print("Should exit after one full loop2\n");
    two_opt_full(inst, tour);
    print_tour(inst, tour);

    inst.plot();
    return EXIT_SUCCESS;
}