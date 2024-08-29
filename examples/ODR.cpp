#include "Seiriakos.hpp"
#include <array>
#include <string>

void ODR()
{
  struct Data final
  {
    std::array<float, 5> a;
    std::string          b;

    stz::trivial_serialization(a, b)
  } to_serialize;

  
  auto binary       = stz::serialize(to_serialize);
  auto deserialized = stz::deserialize<Data>(binary.data(), binary.size());
}