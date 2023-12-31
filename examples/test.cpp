#include "Seiriakos.hpp"
#include "../include/Chronometro.hpp"
#include <iostream>
#include <cstdint>
#include <string>
#include <array>
#include <cerrno>

// serializable data structure
struct SDS final : public Seiriakos::Serializable
{
  uint16_t a;

  SEIRIAKOS_SEQUENCE(a);
};

struct Something final : public Seiriakos::Serializable
{
  std::array<uint8_t, 5>                  a;
  std::string                             b;
  SDS                                     c;
  std::tuple<int, std::tuple<int, float>> d;
  std::string                             e;
  std::map<int, std::string>              f;
  std::vector<double>                     g;

  SEIRIAKOS_SEQUENCE(a, b, c, d, e, f, g);
};

int main()
{
  using namespace Chronometro;

  Something something, decoded;
  std::vector<uint8_t> serialized;
  
  something.a = {'h', 'e', 'a', 'd', '\0'};
  something.b = "allo ceci est une string de taille moyennement grande";
  something.c.a = 0xBEEF;
  something.d = {2, {7, 3.1415}};
  something.e = "bye";
  something.f = {{1, "one"}, {2, "two"}, {3, "three"}};
  something.g = {0, 1, 2, 3, 4, 5, 6, 7};

loop:
  CHRONOMETRO_MEASURE(1)
  serialized = something.serialize();

  std::cout << "a:   " << something.a.data() << '\n';
  std::cout << "b:   " << something.b << '\n';
  std::cout << "c:   " << something.c.a << '\n';
  std::cout << "d0:  " << std::get<0>(something.d) << '\n';
  std::cout << "d10: " << std::get<0>(std::get<1>(something.d)) << '\n';
  std::cout << "d11: " << std::get<1>(std::get<1>(something.d)) << '\n';
  std::cout << "e:   " << something.e << '\n';
  std::cout << "f:   " << '\n';
  for (auto& pair : something.f)
  {
    std::cout << "     " << pair.first << " " << pair.second << ' ';
  }
  std::cout << '\n';
  std::cout << "g:   " << '\n' << "     ";
  for (auto val : something.g)
  {
    std::cout << val << ' ';
  }
  std::cout << '\n';

  // std::cout << Seiriakos::bytes_as_cstring(serialized.data(), serialized.size()) << '\n';
  
  CHRONOMETRO_MEASURE(1)
  decoded.deserialize(serialized.data(), serialized.size());

  std::cout << "a:   " << decoded.a.data() << '\n';
  std::cout << "b:   " << decoded.b << '\n';
  std::cout << "c:   " << decoded.c.a << '\n';
  std::cout << "d0:  " << std::get<0>(decoded.d) << '\n';
  std::cout << "d10: " << std::get<0>(std::get<1>(decoded.d)) << '\n';
  std::cout << "d11: " << std::get<1>(std::get<1>(decoded.d)) << '\n';
  std::cout << "e:   " << decoded.e << '\n';
  std::cout << "f:   " << '\n';
  for (auto& pair : decoded.f)
  {
    std::cout << "     " << pair.first << " " << pair.second << ' ';
  }
  std::cout << '\n';
  std::cout << "g:   " << '\n' << "     ";
  for (auto val : decoded.g)
  {
    std::cout << val << ' ';
  }
  std::cout << '\n';

  std::cin.get();
  goto loop;
}