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
    while (!(X))                 \
    {                            \
        __builtin_unreachable(); \
    }

#elif defined(_MSC_VER)
#define ASSUME(X) __assume(X)
#endif

#define MASK 0x80000000

#define flag_clear(X, Y) \
    (X[(Y)] & ~MASK)

#define flag_off(X, Y) \
    (X[(Y)] = flag_clear((X), (Y)))

#define flag_on(X, Y) \
    (X[(Y)] |= MASK)

#define flag_test(X, Y) \
    (X[(Y)] & MASK)

void print_tour(Instance &inst, std::vector<int> &tour)
{
    int length = 0;
    //println!("{:2?}", tour);
    for (int v = 1; v < (inst.size()); ++v)
    {
        int l = inst.dist(tour[v - 1], tour[v]);
        length += l;
        //print!("{} {} ", l, if v < inst.size() - 1 { "+" } else { "=" });
    }
    fmt::print("Lengh: {}; \n", length);
}

std::vector<int> nn_tour(Instance &inst)
{
    int n = inst.size();
    std::vector<int> tour(n);
    for (int v = 0; v < n; ++v)
    {
        tour[v] = v;
    }

    for (int i = 0; i < n - 2; ++i)
    {
        auto nearest = i + 1;
        auto d_nearest = inst.dist(tour[i], tour[(i + 1)]);
        for (int j = i + 2; j < n; ++j)
        {
            auto const d_candidate = inst.dist(tour[i], tour[j]);
            if (d_candidate < d_nearest)
            {
                nearest = j;
                d_nearest = d_candidate;
            }
        }
        auto const temp = tour[(i + 1)];
        tour[(i + 1)] = tour[nearest];
        tour[nearest] = temp;
    }
    return tour;
}

void flip_internal(int *__restrict buf1, int *__restrict buf2, int size)
{
    for (int i = 0; i < size; ++i)
    {
        int temp = buf1[i];
        buf1[i] = buf2[size - 1 - i];
        buf2[size - 1 - i] = temp;
    }
}

void flip_n(std::vector<int> &tour, int beg, int end, int n)
{
    auto const mid = end + 1 - n;
    flip_internal(tour.data() + beg, tour.data() + mid, n);
}

void two_opt_apply(Instance &inst, std::vector<int> &tour, int i, int j)
{
    flip_n(tour, i + 1, j, (j - i) / 2);
}

int two_opt_gain(Instance &inst, std::vector<int> &tour, int i, int j)
{
    auto const n = inst.size();
    auto const jnext = j != n - 1 ? j + 1 : 0;
    auto const added = inst.dist(flag_clear(tour, i), flag_clear(tour, j)) + inst.dist(flag_clear(tour, i + 1), flag_clear(tour, jnext));
    auto const removed = inst.dist(flag_clear(tour, i), flag_clear(tour, i + 1)) + inst.dist(flag_clear(tour, j), flag_clear(tour, jnext));
    return removed - added;
}

void two_opt_full(Instance &inst, std::vector<int> &tour)
{
    auto const n = inst.size();
    auto skip_flag_test = 2;
    auto ext_pass_gain = 0;
    do
    {
        fmt::print("loop 1\n");
        ext_pass_gain = 0;
        auto pass_gain = 0;
        do
        {
            fmt::print("  loop 2\n");
            pass_gain = 0;
            for (int i = 0; i < (n - 1); ++i)
            {
                if (!(skip_flag_test > 0))
                {
                    auto const cond = !flag_test(tour, i) || !flag_test(tour, i + 1);
                    flag_off(tour, i);
                    if (i + 1 == n - 1)
                        flag_off(tour, i + 1);
                    if (cond)
                        continue;
                }

                auto i_gain = 0;
                auto j = i + 2;
                while (j < n)
                {
                    auto const jnext = j != n - 1 ? j + 1 : 0;
                    auto const gain = two_opt_gain(inst, tour, i, j);
                    if (gain > 0)
                    {
                        i_gain += gain;
                        two_opt_apply(inst, tour, i, j);
                        flag_on(tour, j);
                        flag_on(tour, jnext);
                    }
                    j += 1;
                }

                if (i_gain > 0)
                {
                    flag_on(tour, i);
                    flag_on(tour, i + 1);
                    pass_gain += i_gain;
                }
            }
            if (pass_gain > 0)
                ext_pass_gain += pass_gain;
            skip_flag_test = 0;

        } while (pass_gain > 0);
        skip_flag_test = 1;

    } while (ext_pass_gain > 0);
}

int main(int argc, char **args)
{
    if (argc < 1)
    {
        fmt::print("Needs instance path.\n");
        abort();
    }

    Instance inst(args[1]);

    fmt::print(
        "Name: {}, Dist: {}, Size: {}\n",
        inst.get_name(),
        inst.get_dist_type(),
        inst.size());

    fmt::print("NN tour:\n");
    auto tour = nn_tour(inst);
    print_tour(inst, tour);

    fmt::print("Applying 2Opt:\n");
    two_opt_full(inst, tour);
    fmt::print("Should exit after one full loop2\n");
    two_opt_full(inst, tour);
    print_tour(inst, tour);

    return EXIT_SUCCESS;
}