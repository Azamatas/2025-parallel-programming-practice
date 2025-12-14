#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <thread>
#include <string>

static unsigned num_threads() {
    unsigned T = std::thread::hardware_concurrency();
    if (T == 0) T = 4;
    if (T > 32) T = 32;
    return T;
}

std::vector<size_t> read_array() {
    size_t length, a, b, p;
    std::cin >> length >> a >> b >> p;
    std::vector<size_t> result(length);
    if (length == 0) return result;

    result[0] = a % p;
    for (size_t i = 1; i < result.size(); ++i) {
        result[i] = (result[i - 1] * a + b) % p;
    }
    return result;
}

static void parallel_sort(std::vector<size_t>& a) {
    const size_t n = a.size();
    if (n <= 1) return;

    unsigned T = num_threads();
    if (T <= 1 || n < 1'0000) { 
        std::sort(a.begin(), a.end());
        return;
    }
    if ((size_t)T > n) T = (unsigned)n;

    // Sort initial runs in parallel
    std::vector<size_t> cuts(T + 1);
    for (unsigned t = 0; t <= T; ++t) cuts[t] = (n * t) / T;

    std::vector<std::thread> threads;
    threads.reserve(T);
    for (unsigned t = 0; t < T; ++t) {
        size_t L = cuts[t], R = cuts[t + 1];
        threads.emplace_back([&a, L, R]() {
            std::sort(a.begin() + (ptrdiff_t)L, a.begin() + (ptrdiff_t)R);
        });
    }
    for (auto& th : threads) th.join();
    // Merge sorted runs in parallel
    std::vector<size_t> tmp(n);
    std::vector<size_t> starts = cuts; // current run boundaries

    bool in_a = true;
    while (starts.size() > 2) {
        std::vector<size_t> next_starts;
        next_starts.reserve((starts.size() + 1) / 2);
        next_starts.push_back(0);
        threads.clear();
        size_t runs = starts.size() - 1;
        size_t pair_count = runs / 2;

        unsigned MT = std::min<unsigned>(num_threads(), (unsigned)std::max<size_t>(1, pair_count));
        if (pair_count == 0) MT = 0;
        
        std::atomic<size_t> next_pair{0};

        auto merge_worker = [&]() {
            while (true) {
                size_t pi = next_pair.fetch_add(1, std::memory_order_relaxed);
                if (pi >= pair_count) break;

                size_t i0 = starts[2 * pi];
                size_t i1 = starts[2 * pi + 1];
                size_t i2 = starts[2 * pi + 2];

                if (in_a) {
                    std::merge(a.begin() + (ptrdiff_t)i0, a.begin() + (ptrdiff_t)i1,
                               a.begin() + (ptrdiff_t)i1, a.begin() + (ptrdiff_t)i2,
                               tmp.begin() + (ptrdiff_t)i0);
                } else {
                    std::merge(tmp.begin() + (ptrdiff_t)i0, tmp.begin() + (ptrdiff_t)i1,
                               tmp.begin() + (ptrdiff_t)i1, tmp.begin() + (ptrdiff_t)i2,
                               a.begin() + (ptrdiff_t)i0);
                }
            }
        };

        for (unsigned t = 0; t < MT; ++t) threads.emplace_back(merge_worker);
        for (auto& th : threads) th.join();
        // If odd number of runs, copy the last one
        if (runs % 2 == 1) {
            size_t L = starts[runs - 1];
            size_t R = starts[runs];
            if (in_a) {
                std::copy(a.begin() + (ptrdiff_t)L, a.begin() + (ptrdiff_t)R,
                          tmp.begin() + (ptrdiff_t)L);
            } else {
                std::copy(tmp.begin() + (ptrdiff_t)L, tmp.begin() + (ptrdiff_t)R,
                          a.begin() + (ptrdiff_t)L);
            }
        }

        for (size_t r = 0; r + 1 < runs; r += 2) {
            next_starts.push_back(starts[r + 2]); 
        }
        if (runs % 2 == 1) next_starts.push_back(starts.back());

        starts.swap(next_starts);
        in_a = !in_a;
    }

    if (!in_a) {
        std::copy(tmp.begin(), tmp.end(), a.begin());
    }
}

int main() {
    auto array = read_array();
    parallel_sort(array);

    size_t k;
    std::cin >> k;
    for (size_t i = k - 1; i < array.size(); i += k) {
        std::cout << array[i] << ' ';
    }
    std::cout << "\n";
    return 0;
}
