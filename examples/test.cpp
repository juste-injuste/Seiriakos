#define SEIRIAKOS_LOGGING
#include "../include/Seiriakos.hpp"
#define CHRONOMETRO_WARNINGS
#include "../include/Chronometro.hpp"
#include <iostream>
#include <cstdint>
#include <string>
#include <array>

// scuffed sleep function to demonstrate the basic usage of the library
void sleep_for_ms(std::chrono::high_resolution_clock::rep ms)
{
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  while (std::chrono::nanoseconds(std::chrono::high_resolution_clock::now()-start).count() < ms*1000000);
}

// serializable data structure
struct SDS final : public Seiriakos::Serializable
{
  uint16_t a;

  SEIRIAKOS_SEQUENCE(a);
};

struct Something final : public Seiriakos::Serializable
{
  std::array<uint8_t, 15500> a;
  std::string b = "allo";
  SDS c;

  SEIRIAKOS_SEQUENCE(a, b, c);
};

int main()
{
  using namespace Chronometro;

  Something something;
  std::vector<uint8_t> serialized;
  Something decoded;

loop:
  CHRONOMETRO_MEASURE()
  serialized = something.serialize();
  
  CHRONOMETRO_MEASURE()
  decoded.deserialize(serialized.data(), serialized.size());

//   CHRONOMETRO_MEASURE_LAPS(10, "iteration %# took: %ms")
//   {
//     sleep_for_ms(100);
//   }
  std::cin.get();
  goto loop;
}