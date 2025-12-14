#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>

std::istream& operator>>(std::istream& in, __int128& value) {
    std::string s;
    in >> s;
    value = 0;
    bool negative = false;
    size_t i = 0;
    if (!s.empty() && s[0] == '-') {
        negative = true;
        i = 1;
    }
    for (; i < s.size(); ++i) {
        value = value * 10 + (s[i] - '0');
    }
    if (negative) value = -value;
    return in;
}

std::ostream& operator<<(std::ostream& out, __int128 value) {
    if (value == 0) {
        out << '0';
        return out;
    }
    std::string s;
    bool negative = false;
    if (value < 0) {
        negative = true;
        value = -value;
    }
    while (value > 0) {
        int digit = (int)(value % 10);
        s.push_back(char('0' + digit));
        value /= 10;
    }
    if (negative) s.push_back('-');
    std::reverse(s.begin(), s.end());
    out << s;
    return out;
}

int main() {
    __int128 n;
    std::cin >> n;
    if (n <= 1) return 0;
    std::vector<__int128> factors;
    unsigned T = std::thread::hardware_concurrency();
    if (T == 0) T = 4;
    if (T > 32) T = 32;
    long long end = (long long)std::sqrt((long double)n);
    if (end < 2) end = 2;
    std::mutex m; 
    auto worker = [&](unsigned tid) {
        for (long long p = 2 + (long long)tid; p <= end; p += (long long)T) {
            __int128 current_n = n;
            if (current_n <= 1) return;
            if (current_n % p != 0) continue;
            std::lock_guard<std::mutex> lock(m);
            while (n % p == 0) {
                factors.push_back((__int128)p);
                n /= p;
            }
        }
    };
    std::vector<std::thread> threads;
    threads.reserve(T);
    for (unsigned t = 0; t < T; ++t) threads.emplace_back(worker, t);
    for (auto& th : threads) th.join();

    if (n > 1) factors.push_back(n);

    for (const auto& factor : factors) {
        std::cout << factor << ' ';
    }
    std::cout << '\n';
    return 0;
}
