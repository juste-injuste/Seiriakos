// #define SRZ_DEBUGGING
#define SRZ_UNSAFE
#define SRZ_NOT_THREADSAFE
#define SRZ_FIXED_LENGHT
#include "Seiriakos.hpp"
#include "../include/Chronometro.hpp"
#include <iostream>
#include <cstdint>
#include <string>
#include <array>

// serializable data structure
struct SDS final : public srz::Serializable
{
  uint16_t a;

  SRZ_SERIALIZATION_SEQUENCE(a);
};

struct Something final : public srz::Serializable
{
  std::array<uint8_t, 5>                  a;
  std::string                             b;
  SDS                                     c;
  std::tuple<int, std::tuple<int, float>> d;
  std::string                             e;
  std::map<int, std::string>              f;
  std::vector<double>                     g;
  std::bitset<4>                          h;
  std::atomic_int i;
  std::ratio<4, 3> j;
  std::stack<int> k;
  std::queue<int> l;
  std::forward_list<int> m;
  std::priority_queue<std::string> n;

  SRZ_SERIALIZATION_SEQUENCE(a, b, c, d, e, f, g, h, i, j, k, l, m, n);
};

int main()
{
  Something something, decoded;
  std::vector<uint8_t> serialized;
  
  something.a = {'h', 'e', 'a', 'd', '\0'};
  something.b = "allo ceci est une string de taille moyennement grande";
  something.c.a = 0xBEEF;
  something.d = {2, {7, 3.1415}};
  something.e = "bye";
  something.f = {{1, "one"}, {2, "two"}, {3, "three"}};
  something.g = {0, 1, 1, 1, 1, 0, 1, 1};
  something.h[0] = true;
  something.h[1] = false;
  something.h[2] = true;
  something.h[3] = false;
  something.k.push(1);
  something.k.push(2);
  something.k.push(3);

loop:
  CHZ_MEASURE(10, "iteration %# took %ms")
  CHZ_LOOP_FOR(100000)
  serialized = something.serialize();

  std::cout << serialized.size();

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

  // std::cout << Seiriakos::bytes_as_cstring(serialized.data(), serialized.size()) << '\n';
  
  CHZ_MEASURE(10, "iteration %# took %ms")
  CHZ_LOOP_FOR(100000)
  decoded.deserialize(serialized.data(), serialized.size());

  std::priority_queue<int> pq;
  srz::_impl::_serialization_implementation(pq);

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