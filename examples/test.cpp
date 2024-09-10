#define STZ_DEBUGGING
#define STZ_UNSAFE
#define STZ_NOT_THREADSAFE
// #define STZ_FIXED_LENGHT
// #define STZ_ALLOW_CONSTCAST
#include "../include/Seiriakos.hpp"

struct gps_position
{
  gps_position() = default;
  gps_position(const int degrees_, const int minutes_, const float seconds_)
    : _degrees(degrees_)
    , _minutes(minutes_)
    , _seconds(seconds_)
  {}

  stz::serialization_methods()

private:
  int   _degrees = {};
  int   _minutes = {};
  float _seconds = {};

  stz::serialization_sequential(_degrees, _minutes, _seconds)
};


int main()
{
  const gps_position position(35, 59, 24.567f);
  gps_position       aquired_position;

  auto binary = position.serialize();
  aquired_position.deserialize(binary.data(), binary.size());
}