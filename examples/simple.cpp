#include <string>
#include <iostream>
// #define STZ_DEBUGGING
// #define STZ_ALLOW_CONSTCAST
#include "../include/Seiriakos.hpp"

struct Building
{
  Building() : _uuid([]{static int uuid = 0; return ++uuid; }()) {}

  virtual
  void info() = 0;

  int  uuid() { return _uuid; }

private:
  int _uuid;
  stz::trivial_serialization(_uuid)
};

struct Person
{
  Person() = default;
  Person(const char* string) : name(string) {}

  std::string name = "n/a";

  stz::trivial_serialization(name)
};

struct Bar : private Building
{
  Bar() = default;
  Bar(Person person, int16_t count) : owner(person), items(count) {}

  void info() override
  {
    std::cout << "  id:    " << uuid()     << '\n';
    std::cout << "  owner: " << owner.name << '\n';
    std::cout << "  items: " << items      << '\n';
  }

  Person  owner;
  int16_t items;

  stz::serialization_methods()
  stz::trivial_serialization(stz::abstracted<Building>(this), items, owner)
};

int main()
{
  Bar bar("John Pilcrow", 13);

  auto binary   = bar.serialize();
  auto bar_copy = stz::deserialize<Bar>(binary.data(), binary.size());

  std::cout << "\noriginal bar:\n";
  bar.info();

  std::cout << "\ndeserialized bar:\n";
  bar_copy.info();

  std::cout << "\nrandom bar:\n";
  Bar().info();

  std::cout << "\nserialized hex: " << stz::hex_string(binary.data(), binary.size()) << '\n';
}