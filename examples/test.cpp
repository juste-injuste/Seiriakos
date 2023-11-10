//#define SEIRIAKOS_LOGGING
#define SEIRIAKOS_LOGGING
#include "../include/logger.hpp"
#include "../include/Seiriakos.hpp"
#include "../include/Chronometro.hpp"
#include <iostream>
#include <cstdint>
#include <iostream>
#include <sstream>

// scuffed sleep function to demonstrate the basic usage of the library
void sleep_for_ms(std::chrono::high_resolution_clock::rep ms)
{
  std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
  while (std::chrono::nanoseconds(std::chrono::high_resolution_clock::now()-start).count() < ms*1000000);
}

struct Something final : public Seiriakos::Serializable
{
  std::array<uint8_t, 155> a;
  std::string b = "allo";

  SEIRIAKOS_SEQUENCE(a, b);
};

void foo()
{}

int main()
{
  using namespace Chronometro;

  Something something;
  std::vector<uint8_t> serialized;
  Something decoded;
loop:
  CHRONOMETRO_MEASURE(1)
  serialized = something.serialize();
  
  CHRONOMETRO_MEASURE(1)
  decoded.deserialize(serialized.data(), serialized.size());

  CHRONOMETRO_MEASURE_LAPS(10, "iteration %# took: %ms")
  {
    sleep_for_ms(100);
  }

  std::cin.get();

  goto loop;
}