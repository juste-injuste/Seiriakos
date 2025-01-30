#include <string>
#include <iostream>
// #define STZ_DEBUGGING
// #define STZ_UNSAFE
// #define STZ_NOT_THREADSAFE
// #define STZ_FIXED_LENGHT
#include "../include/Seiriakos.hpp"
#include "../include/Chronometro.hpp"

struct Building
{
  Building();

  int uuid() const;

  virtual
  void info() const = 0;

private:
  const int _uuid;
  stz::serialization_sequential(stz::as_mutable(_uuid))
};

struct Color
{
  enum Basic : uint8_t { Black, Red, Green, Yellow, Blue,  Magenta, Cyan,  White, Gray };

  Color(const Basic basic_);
  Color(uint8_t R, uint8_t G, uint8_t B);

private:
  enum class Type : uint8_t { RGB, BASIC } _type;
  union Data
  {
    uint8_t _rgb[3] = {};
    Basic   _basic;
  } _data;

  struct Legacy { uint32_t _hex; } _legacy;

  stz::serialization_procedural
  (
    serializer.version = 3;

    if (serializer.version < 2)
    {
      serializer <= _legacy._hex;
    }
    else
    {
      serializer <= _type;
      
      if      (_type == Type::BASIC) serializer <= _data._basic;
      else if (_type == Type::RGB)   serializer <= _data._rgb;
    }
  )
};

struct Chair
{
  enum Type : uint8_t { Recliner, Loveseat, Banquette, Stool };
  Chair() = default;
  Chair(Type type_, Color color_) : type(type_), color(color_) {}

  Type  type  = Type::Stool;
  Color color = Color::Basic::Black;

  stz::serialization_sequential(type, color);
};

struct Person
{
  Person() = default;
  Person(const char* string, Color color) : name(string), hair(color) {}

  std::string name = "n/a";
  Color       hair = Color::Basic::Black;
  unsigned    age  : 9;

  stz::serialization_sequential(name, hair, stz::bitfield<9>(age))
};

struct Bar : private Building
{
  Bar() = default;

  void info() const override
  {
    std::cout << "  id:    " << uuid()       << '\n';
    std::cout << "  owner: " << owner.name   << '\n';
    std::cout << "  seats: " << seats.size() << '\n';
  }

  Person             owner;
  std::vector<Chair> seats;

  stz::serialization_sequential(stz::base_type<Building>(this), seats, owner)
};

int main()
{

  Person     John("John Pilcrow"  , Color( 65, 53, 50));
  Chair     stool(Chair::Stool    , Color::Black      );
  Chair banquette(Chair::Banquette, Color(113, 47, 62));

  Bar bar;

  bar.owner = John;

  stz::loop_n_times(16)
  {
    bar.seats.push_back(stool);
  };

  stz::loop_n_times(8)
  {
    bar.seats.push_back(banquette);
  };

  auto binary = stz::serialize(bar);
  auto copy   = stz::deserialize<Bar>(binary.data(), binary.size());

  std::cout << "\noriginal bar:\n";
  bar.info();

  std::cout << "\ndeserialized bar:\n";
  copy.info();

  std::cout << "\nrandom bar:\n";
  Bar().info();

  std::cout << "\nserialized hex: " << stz::hex_string(binary.data(), binary.size()) << '\n';
  std::cout << "\nbyte count:     " << binary.size() << '\n';
}

Building::Building()
  : _uuid([]{ static int uuid = 0; return ++uuid; }())
{}

int Building::uuid() const
{
  return _uuid;
}

Color::Color(uint8_t R, uint8_t G, uint8_t B)
  : _type(Type::RGB)
{
  _data._rgb[0] = R; _data._rgb[1] = G; _data._rgb[2] = B;
}

Color::Color(const Basic basic_)
  : _type(Type::BASIC)
{
  _data._basic = basic_;
}