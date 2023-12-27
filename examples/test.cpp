// #define SEIRIAKOS_LOGGING
#include "../include/Seiriakos.hpp"
#define CHRONOMETRO_WARNINGS
#include "../include/Chronometro.hpp"
#include <iostream>
#include <cstdint>
#include <string>
#include <array>
#include <cerrno>

// scuffed sleep function to demonstrate the basic usage of the library
void sleep_for_ms(std::chrono::high_resolution_clock::rep ms)
{
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  while (std::chrono::nanoseconds(std::chrono::high_resolution_clock::now()-start).count() < ms*1000000);
}

// serializable data structure
struct SDS final : public Seiriakos::Serializable
{
  uint16_t a = 0xFFFF;

  SEIRIAKOS_SEQUENCE(a);
};

struct Something final : public Seiriakos::Serializable
{
  std::array<uint8_t, 555> a = {'h', 'e', 'a', 'd', '\0'};
  std::string b = "allo";
  SDS c;
  std::tuple<int, std::tuple<int, float>> d = {2, {7, 3.1415}};
  std::string e = "bye";

  std::map<int, std::string> f = {{1, "one"}, {2, "two"}, {3, "three"}};

  SEIRIAKOS_SEQUENCE(a, b, c, d, e);
};

int main()
{
  using namespace Chronometro;

  Something something;
  std::vector<uint8_t> serialized;
  Something decoded;

loop:
  CHRONOMETRO_MEASURE(1000000)
  serialized = something.serialize();

  std::cout << "a:   " << something.a.data() << '\n';
  std::cout << "b:   " << something.b << '\n';
  std::cout << "d0:  " << std::get<0>(something.d) << '\n';
  std::cout << "d10: " << std::get<0>(std::get<1>(something.d)) << '\n';
  std::cout << "d11: " << std::get<1>(std::get<1>(something.d)) << '\n';
  std::cout << "e:   " << something.e << '\n';
  std::cout << "f:   " << '\n';
  for (auto& pair : something.f)
  {
    std::cout << "     " << pair.first << " " << pair.second << '\n';
  }

  // std::cout << Seiriakos::bytes_as_cstring(serialized.data(), serialized.size()) << '\n';
  
  CHRONOMETRO_MEASURE(1000000)
  decoded.deserialize(serialized.data(), serialized.size());

  std::cout << "a:   " << decoded.a.data() << '\n';
  std::cout << "b:   " << decoded.b << '\n';
  std::cout << "d0:  " << std::get<0>(decoded.d) << '\n';
  std::cout << "d10: " << std::get<0>(std::get<1>(decoded.d)) << '\n';
  std::cout << "d11: " << std::get<1>(std::get<1>(decoded.d)) << '\n';
  std::cout << "e:   " << decoded.e << '\n';
  std::cout << "f:   " << '\n';
  for (auto& pair : decoded.f)
  {
    std::cout << "     " << pair.first << " " << pair.second << '\n';
  }

  std::cin.get();
  goto loop;
}