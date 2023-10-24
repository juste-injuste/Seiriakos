#include <iostream>

int main() {
    // You can use Unicode characters for box drawing
    for (int i = 0; i < 256; ++i)
        std::cout << i << ": " << char(i) << '\n';
    return 0;
}
