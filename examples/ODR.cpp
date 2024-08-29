#include "Seiriakos.hpp"
#include <array>
#include <string>

void ODR()
{
  struct Data final : public stz::Serializable
  {
    std::array<float, 5> a;
    std::string          b;

    STZ_TRIVIAL_SERIALIZATION(a, b)
  } to_serialize;

  auto binary       = stz::serialize(to_serialize);
  auto deserialized = stz::deserialize<Data>(binary.data(), binary.size());
}