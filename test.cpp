#include <iostream>
#include <cmath>

int main() {
    uint64_t size = UINT32_MAX + 10ull;

    uint8_t bytes_used;
    if (size <= UINT8_MAX)
    bytes_used = 1;
    else if (size <= UINT16_MAX)
    bytes_used = 2;
    else if (size <= UINT32_MAX)
    bytes_used = 4;
    else bytes_used = 8;

    uint8_t bytes_test = 8 - (size <= UINT8_MAX) - 2*(size <= UINT16_MAX) - 4*(size <= UINT32_MAX);

    std::cout << "true: " << int(bytes_used) << '\n';
    std::cout << "test: " << int(bytes_test) << '\n';
    return 0;
}