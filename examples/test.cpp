#define SEIRIAKOS_LOGGING
#include "../include/Seiriakos.hpp"
#include <iostream>
#include <cstdint>
#include <iostream>
#include <sstream>

struct Something final : public Seiriakos::Serializable
{
  std::map<std::string, int> a;
  uint32_t b;
  
  std::string display(const char* name)
  {
    std::stringstream stream;
    stream << name << ":\n";
    stream << "  a:\n";
    for (const auto key_value : a)
    {
      stream << "    key: " << key_value.first << ", value: " << key_value.second << '\n';
    }
    stream << "  b:\n";
    stream << "    " << b << '\n';
    return stream.str();
  }

  SEIRIAKOS_SEQUENCE(a, b);
};

int main()
{
  Something something;
  something.a.insert({"test", 1});
  something.a.insert({"allo", 2});
  something.a.insert({"byebye", 3});
  something.a.insert({"f", 54});
  something.a.insert({"f", 32});
  // something.a.insert("test");
  // something.a.insert("allo");
  // something.a.insert("byebye");
  // something.a.insert("f");
  // something.a.insert("f");
  something.b = 0xDEADBEEF;

  std::cout << something.display("something");
  
  std::vector<uint8_t> serialized = something.serialize();

  Seiriakos::print_bytes(serialized.data(), serialized.size());

  Something decoded;
  decoded.deserialize(serialized.data(), serialized.size());

  std::cout << decoded.display("something decoded");

}