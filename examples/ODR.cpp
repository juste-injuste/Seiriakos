#include "Seiriakos.hpp"

int test()
{
  struct SomeStruct final : public srz::Serializable
  {
    std::array<float, 5> a;
    std::string          b;

    SRZ_SERIALIZATION_SEQUENCE
    (
      serialization(a, b);
    );
  } to_serialize, deserialized;

  auto serialized_data = to_serialize.serialize();
  
  deserialized.deserialize(serialized_data.data(), serialized_data.size());
  
  return 0;
}