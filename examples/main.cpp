#include "../include/Seiriakos.hpp"
#include <iostream>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

// serializable data structure
struct SDS final : public Seiriakos::Serializable
{
  uint16_t a;
  uint64_t b;
  uint64_t c;
  
  std::string display(const char* name)
  {
    std::stringstream stream;
    stream << "  " << name << ":\n";  
    stream << "    a: " << a   << '\n';
    stream << "    b: " << b   << '\n';
    stream << "    c: " << c   << '\n';
    return stream.str();
  }

  SEIRIAKOS_SEQUENCE(a);
};

struct Something final : public Seiriakos::Serializable
{
  uint16_t a;
  uint8_t  b;
  uint32_t c;
  SDS      d;
  uint64_t e;
  
  std::string display(const char* name)
  {
    std::stringstream stream;
    stream << name << ":\n";  
    stream << "  a: " << a   << '\n';
    stream << "  b: " << b   << '\n';
    stream << "  c: " << c   << '\n';
    stream << d.display("d");
    stream << "  e: " << e   << '\n';
    return stream.str();
  }

  SEIRIAKOS_SEQUENCE(a, b, c, d, e);
};

int main()
{
  Something something;
  something.a   = 0x55;
  something.b   = 0x72;
  something.c   = 0x14;
  something.d.a = 0x5500;
  something.d.b = 0x7777777777777777;
  something.d.c = 0x3333333333333333;
  something.e   = 0xdeadbeef12345678;

  std::cout << something.display("something");
  
  std::vector<uint8_t> serialized = something.serialize();

  Seiriakos::print_bytes(serialized.data(), serialized.size());

  // Something decoded;
  // decoded.deserialize(serialized.data(), serialized.size());

  // std::cout << decoded.display("something decoded");
}