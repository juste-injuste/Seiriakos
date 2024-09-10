
// #define STZ_DEBUGGING
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "../include/Seiriakos.hpp"


struct gps_position
{
  gps_position() = default;
  gps_position(const int degrees_, const int minutes_, const float seconds_)
    : _degrees(degrees_)
    , _minutes(minutes_)
    , _seconds(seconds_)
  {}
  
private:
  friend std::ostream& operator<<(std::ostream&, const gps_position&);

  int   _degrees = {};
  int   _minutes = {};
  float _seconds = {};

  stz::trivial_serialization(_degrees, _minutes, _seconds)
};

struct bus_stop
{
  bus_stop() = default;
  virtual ~bus_stop() = default;

  virtual std::string description() const = 0;

protected:
  friend std::ostream& operator<<(std::ostream& os, const bus_stop& gp);
  bus_stop(const gps_position& latitude_, const gps_position& longitude_)
    : _latitude(latitude_)
    , _longitude(longitude_)
  {}

  gps_position _latitude;
  gps_position _longitude;

  stz::trivial_serialization(_latitude, _longitude)
};

struct bus_stop_corner : public bus_stop
{
  bus_stop_corner() = default;
  bus_stop_corner(
    const gps_position& latitude_, const gps_position& longitude_,
    const std::string&  street1_,  const std::string&  street2_
  )
    : bus_stop(latitude_, longitude_)
    , street1(street1_)
    , street2(street2_)
  {}

private:
  std::string street1;
  std::string street2;

  virtual std::string description() const
  {
    return street1 + " and " + street2;
  }

  stz::trivial_serialization
  (
    stz::base_type<bus_stop>(this), street1, street2
  )
};

struct bus_stop_destination : public bus_stop
{
  bus_stop_destination() = default;
  bus_stop_destination(const gps_position& latitude_, const gps_position& longitude_, const std::string& name_)
    : bus_stop(latitude_, longitude_)
    , _name(name_)
  {}

private:
  std::string _name;

  virtual std::string description() const
  {
    return _name;
  }

  stz::serialization_sequence
  (
    // serialize <= dynamic_cast<bus_stop>(*this);
    serialize <= _name;
  )
};

struct bus_route
{
  bus_route() = default;

  void append(bus_stop* bus_stop_)
  {
    _stops.insert(_stops.end(), bus_stop_);
  }

private:
  friend std::ostream& operator<<(std::ostream&, const bus_route&);

  std::list<bus_stop*> _stops;

  stz::serialization_sequence
  (
    // ar.register_type(static_cast<bus_stop_corner*>(nullptr));
    // ar.register_type(static_cast<bus_stop_destination*>(nullptr));
    serialize <= _stops;
  )
};

struct bus_schedule
{
  struct trip_info
  {
    trip_info() = default;
    trip_info(int hour_, int minute_, const std::string& driver_)
      : _hour(hour_)
      , _minute(minute_)
      , _driver(driver_)
    {}

    int         _hour;
    int         _minute;
    std::string _driver;

    friend std::ostream& operator<<(std::ostream&, const bus_schedule::trip_info&);

    stz::serialization_sequence
    (
      // if(file_version >= 2)
      //   ar&  driver;

      // serialize <= _driver;

      serialize <= _hour, _minute;
    )
  };

  bus_schedule() = default;

  void append(const std::string& driver_, const int hour_, const int minute_, bus_route* const bus_route_)
  {
    _schedule.insert(_schedule.end(), std::pair<trip_info, bus_route*>(trip_info(hour_, minute_, driver_), bus_route_));
  }

private:
  friend std::ostream& operator<<(std::ostream&, const bus_schedule&);

  std::list<std::pair<trip_info, bus_route*>> _schedule;
  
  stz::trivial_serialization(_schedule)
};

int main()
{
  bus_schedule schedule_to_serialize;

  std::unique_ptr<bus_stop> bs0{new bus_stop_corner
  (
    gps_position(34, 135, 52.560f),
    gps_position(134, 22, 78.30f),
    "24th Street", "10th Avenue"
  )};

  std::unique_ptr<bus_stop> bs1{new bus_stop_corner
  (
    gps_position(35, 137, 23.456f),
    gps_position(133, 35, 54.12f),
    "State street", "Cathedral Vista Lane"
  )};

  std::unique_ptr<bus_stop> bs2{new bus_stop_destination
  (
    gps_position(35, 136, 15.456f),
    gps_position(133, 32, 15.300f),
    "White House"
  )};

  std::unique_ptr<bus_stop> bs3{new bus_stop_destination
  (
    gps_position(35, 134, 48.789f),
    gps_position(133, 32, 16.230f),
    "Lincoln Memorial"
  )};

  bus_route route0;
  route0.append(bs0.get());
  route0.append(bs1.get());
  route0.append(bs2.get());

  schedule_to_serialize.append("bob",    6, 24, &route0);
  schedule_to_serialize.append("bob",    9, 57, &route0);
  schedule_to_serialize.append("alice", 11, 02, &route0);

  bus_route route1;
  route1.append(bs3.get());
  route1.append(bs2.get());
  route1.append(bs1.get());

  schedule_to_serialize.append("ted",    7, 17, &route1);
  schedule_to_serialize.append("ted",    9, 38, &route1);
  schedule_to_serialize.append("alice", 11, 47, &route1);

  // std::cout << "original schedule" << schedule_to_serialize;
  stz::seiriakos::_impl::_srz_impl(route0);

  auto binary       = stz::serialize(schedule_to_serialize);
  auto new_schedule = stz::deserialize<bus_schedule>(binary.data(), binary.size());

  // and display
  // std::cout << "\nrestored schedule" << new_schedule;
}

std::ostream& operator<<(std::ostream& ostream_, const gps_position& gps_position_)
{
  ostream_ << ' ' << gps_position_._degrees;
  ostream_ << static_cast<unsigned char>(186) << gps_position_._minutes;
  ostream_ << '\'' << gps_position_._seconds << '"';
  return ostream_;
}

std::ostream& operator<<(std::ostream& ostream_, const bus_stop& bs)
{
  ostream_ << bs._latitude;
  ostream_ << bs._longitude;
  ostream_ << ' ' << bs.description();
  return ostream_;
}

std::ostream& operator<<(std::ostream& ostream_, const bus_route& bus_route_)
{  
  for(const auto& stop_ptr : bus_route_._stops)
  {
    ostream_ << '\n' << std::hex << "0x" << stop_ptr << std::dec << ' ' << *stop_ptr;
  }

  return ostream_;
}

std::ostream& operator<<(std::ostream& ostream_, const bus_schedule::trip_info& trip_info_)
{
  return ostream_ << '\n' << trip_info_._hour << ':' << trip_info_._minute << ' ' << trip_info_._driver << ' ';
}

std::ostream& operator<<(std::ostream& ostream_, const bus_schedule& bus_schedule_)
{  
  for(auto& schedule : bus_schedule_._schedule)
  {
    ostream_ << schedule.first << *(schedule.second);
  }

  return ostream_;
}