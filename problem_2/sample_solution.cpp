#include <iostream>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <iomanip>


std::vector<size_t> read_array() {
    size_t length, a, b, p;
    std::cin >> length >> a >> b >> p;
    std::vector<size_t> result(length);
    result[0] = a % p;
    for (size_t i = 1; i < result.size(); ++i) {
        result[i] = (result[i - 1] * a + b) % p;
    }
    return result;
}


int main() {
    auto array = read_array();
    std::sort(array.begin(), array.end());

    size_t k;
    std::cin >> k;
    for (size_t i = k - 1; i < array.size(); i += k) {
        std::cout << array[i] << ' ';
    }
    std::cout << "\n";

    return 0;
}
