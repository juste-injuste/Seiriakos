#include "Seiriakos.hpp"

int ODR()
{
  struct SomeStruct final : public stz::Serializable
  {
    std::array<float, 5> a;
    std::string          b;

    STZ_TRIVIAL_SERIALIZATION(a, b)
  } to_serialize, deserialized;

  auto serialized_data = to_serialize.serialize();
  
  deserialized.deserialize(serialized_data.data(), serialized_data.size());
  
  return 0;
}