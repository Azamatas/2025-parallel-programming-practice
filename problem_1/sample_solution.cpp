#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <iomanip>
#include <thread>
#include <atomic>
#include <string>
#include <limits>

//initialize number of threads
static unsigned num_threads() {
    unsigned T = std::thread::hardware_concurrency();
    if (T == 0) T = 4;
    if (T > 32) T = 32;
    return T;
}

std::vector<std::vector<double>> read_matrix() {
    size_t rows, cols;
    std::cin >> rows >> cols;
§   // Read parameters 
    size_t a, b, x, y, z, p;
    std::cin >> a >> b >> x >> y >> z >> p;

    if (rows == 0 || cols == 0) {
         return {}; }

    std::vector<std::vector<uint64_t>> intermediate(rows, std::vector<uint64_t>(cols));

    uint64_t b_mod = b % p;
    for (size_t i = 0; i < rows; ++i) {
        std::fill(intermediate[i].begin(), intermediate[i].end(), b_mod);
    }
  
    intermediate[0][0] = a % p;
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            if (i == 0 && j == 0) continue;
            
            uint64_t val = b_mod;
            
            if (i > 0 && j > 0) {
                val = (val + (uint64_t)x * intermediate[i-1][j-1]) % p;
            }
            if (i > 0) {
                val = (val + (uint64_t)y * intermediate[i-1][j]) % p;
            }
            if (j > 0) {
                val = (val + (uint64_t)z * intermediate[i][j-1]) % p;
            }
            
            intermediate[i][j] = val;
        }
    }

    // Parallel max scan
    unsigned T = num_threads();
    std::vector<std::thread> threads;
    threads.reserve(T);
    std::vector<uint64_t> local_max(T, 0);

    auto max_worker = [&](unsigned tid) {
        size_t start = (rows * tid) / T;
        size_t end   = (rows * (tid + 1)) / T;
        uint64_t m = 0;
        for (size_t i = start; i < end; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                if (intermediate[i][j] > m) {
                    m = intermediate[i][j];
                }
            }
        }
        local_max[tid] = m;
    };

    for (unsigned t = 0; t < T; ++t) threads.emplace_back(max_worker, t);
    for (auto& th : threads) th.join();

    uint64_t max_value = 0;
    for (unsigned t = 0; t < T; ++t) {
        if (local_max[t] > max_value) {
            max_value = local_max[t];
        }
    }
    
    if (max_value == 0) {
        // Return matrix of zeros
        std::vector<std::vector<double>> result(rows, std::vector<double>(cols, 0.0));
        return result;
    }

    // Parallel normalization
    std::vector<std::vector<double>> result(rows, std::vector<double>(cols));
    threads.clear();

    auto norm_worker = [&](unsigned tid) {
        size_t start = (rows * tid) / T;
        size_t end   = (rows * (tid + 1)) / T;
        double denom = static_cast<double>(max_value);
        for (size_t i = start; i < end; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                result[i][j] = static_cast<double>(intermediate[i][j]) / denom;
            }
        }
    };

    for (unsigned t = 0; t < T; ++t) threads.emplace_back(norm_worker, t);
    for (auto& th : threads) th.join();

    return result;
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    auto left = read_matrix();
    auto right = read_matrix();

    if (left.empty() || right.empty()) {
        std::cerr << "Wrong matrices: empty matrix\n";
        return 1;
    }
    
    if (left[0].size() != right.size()) {
        std::cerr << "Wrong matrices: dimension mismatch\n";
        return 1;
    }

    const size_t left_rows = left.size();
    const size_t left_cols = left[0].size();
    const size_t right_rows = right.size();
    const size_t right_cols = right[0].size();

    std::vector<std::vector<double>> result(left_rows, std::vector<double>(right_cols, 0.0));

    unsigned T = num_threads();
    std::vector<std::thread> threads;
    threads.reserve(T);


    auto mul_worker = [&](unsigned tid) {
        size_t i0 = (left_rows * tid) / T;
        size_t i1 = (left_rows * (tid + 1)) / T;

        for (size_t i = i0; i < i1; ++i) {
            for (size_t k = 0; k < left_cols; ++k) {
                double left_val = left[i][k];
                for (size_t j = 0; j < right_cols; ++j) {
                    result[i][j] += left_val * right[k][j];
                }
            }
        }
    };

    for (unsigned t = 0; t < T; ++t) threads.emplace_back(mul_worker, t);
    for (auto& th : threads) th.join();

    // Output result
    std::cout << left_rows << ' ' << right_cols << "\n";
    for (size_t i = 0; i < left_rows; ++i) {
        for (size_t j = 0; j < right_cols; ++j) {
            std::cout << std::setprecision(12) << std::fixed << result[i][j];
            if (j < right_cols - 1) std::cout << ' ';
        }
        std::cout << "\n";
    }
    return 0;
}