#include <iostream>
#include <type_traits>

// Define a generic has_data_and_size struct

// Example class with data() and size() methods
class MyClass {
  public:
    int* data() { return nullptr; }
    size_t size() { return 0; }
};

int main() {
  std::cout << "MyClass " << (has_data_and_size<MyClass>::value ? "has" : "does not have") << " data() and size() methods." << std::endl;

  return 0;
}
