// #define STZ_DEBUGGING
#define STZ_UNSAFE
#define STZ_NOT_THREADSAFE
// #define STZ_FIXED_LENGHT
#include "Seiriakos.hpp"
#include "../include/Chronometro.hpp"
#include <iostream>
#include <cstdint>
#include <string>
#include <array>

// serializable data structure
struct SDS final : public stz::Serializable
{
  uint16_t a;

  STZ_TRIVIAL_SERIALIZATION(a)
};

struct Something final : public stz::Serializable
{
  std::array<uint8_t, 5>                  a;
  std::string                             b;
  SDS                                     c;
  std::tuple<int, std::tuple<int, float>> d;
  std::string                             e;
  std::map<int, std::string>              f;
  std::vector<double>                     g;
  std::bitset<4>                          h;
  std::atomic_int                         i;
  std::stack<int>                         j;
  std::queue<int>                         k;
  std::forward_list<int>                  l;

  STZ_SERIALIZATION
  (
    serialize <= a, b, c, d, e, f, g, h, i, j, k, l;
  )
};

int main()
{
  Something something, decoded;
  std::vector<uint8_t> binary;
  
  something.a    = {'h', 'e', 'a', 'd', '\0'};
  something.b    = "ceci est une string de taille moyenne.";
  something.c.a  = 0xBEEF;
  something.d    = {2, {7, 3.1415}};
  something.e    = "bye";
  something.f    = {{1, "one"}, {2, "two"}, {3, "three"}};
  something.g    = {0, 1, 1, 1, 1, 0, 1, 1};
  something.h[0] = true;
  something.h[1] = false;
  something.h[2] = true;
  something.h[3] = false;
  something.k.push(1);
  something.k.push(2);
  something.k.push(3);


loop:
  // STZ_MEASURE_BLOCK(10, "iteration %# took %ms")
  STZ_MEASURE_BLOCK(100000)
  binary = something.serialize();
  
  // std::cout << binary.size() << '\n';

  // std::cout << "a:   " << something.a.data() << '\n';
  // std::cout << "b:   " << something.b << '\n';
  // std::cout << "c:   " << something.c.a << '\n';
  // std::cout << "d0:  " << std::get<0>(something.d) << '\n';
  // std::cout << "d10: " << std::get<0>(std::get<1>(something.d)) << '\n';
  // std::cout << "d11: " << std::get<1>(std::get<1>(something.d)) << '\n';
  // std::cout << "e:   " << something.e << '\n';
  // std::cout << "f:   " << '\n';
  // for (auto& pair : something.f)
  // {
  //   std::cout << "     " << pair.first << " " << pair.second << ' ';
  // }
  // std::cout << '\n';
  // std::cout << "g:   " << '\n' << "     ";
  // for (auto val : something.g)
  // {
  //   std::cout << val << ' ';
  // }
  // std::cout << '\n';

  // std::cout << stz::bytes_as_cstring(binary.data(), binary.size()) << '\n';
  
  // STZ_MEASURE_BLOCK(10, "iteration %# took %ms")
  STZ_MEASURE_BLOCK(100000)
  decoded.deserialize(binary.data(), binary.size());

  // std::cout << "a:   " << decoded.a.data() << '\n';
  // std::cout << "b:   " << decoded.b << '\n';
  // std::cout << "c:   " << decoded.c.a << '\n';
  // std::cout << "d0:  " << std::get<0>(decoded.d) << '\n';
  // std::cout << "d10: " << std::get<0>(std::get<1>(decoded.d)) << '\n';
  // std::cout << "d11: " << std::get<1>(std::get<1>(decoded.d)) << '\n';
  // std::cout << "e:   " << decoded.e << '\n';
  // std::cout << "f:   " << '\n';
  // for (auto& pair : decoded.f)
  // {
  //   std::cout << "     " << pair.first << " " << pair.second << ' ';
  // }
  // std::cout << '\n';
  // std::cout << "g:   " << '\n' << "     ";
  // for (auto val : decoded.g)
  // {
  //   std::cout << val << ' ';
  // }
  // std::cout << '\n';

  std::cin.get();
  goto loop;
}