#include "Seiriakos.hpp"

int test()
{
    struct SomeStruct final : public Seiriakos::Serializable
    {
        std::array<float, 5> a;
        std::string          b;

        SEIRIAKOS_SEQUENCE(a, b);
    } to_serialize, deserialized;

    auto serialized_data = to_serialize.serialize();
    
    deserialized.deserialize(serialized_data.data(), serialized_data.size());
    
    return 0;
}