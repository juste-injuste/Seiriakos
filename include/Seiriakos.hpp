/*---author-----------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Seiriakos

-----liscence---------------------------------------------------------------------------------------

MIT License

Copyright (c) 2023 Justin Asselin (juste-injuste)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-----versions---------------------------------------------------------------------------------------

-----description------------------------------------------------------------------------------------

Seiriakos is a simple and lightweight C++11 (and newer) library that allows you serialize and
deserialize objects.

-----inclusion guard------------------------------------------------------------------------------*/
#ifndef SEIRIAKOS_HPP
#define SEIRIAKOS_HPP
//---necessary standard libraries-------------------------------------------------------------------
#include <cstddef>     // for size_t
#include <cstdint>     // for uint8_t
#include <vector>      // for std::vector
#include <type_traits> // for std::is_base_of
#include <iostream>    // for std::cout, std::cerr
#include <iomanip>     // for std::setw, std::setfill, std::hex
#include <ios>         // for std::uppercase 
#if defined(SEIRIAKOS_LOGGING)
#if defined(__STDCPP_THREADS__)
# include <mutex>      // for std::mutex, std::lock_guard
# include <atomic>     // for std::atomic_uint
#endif
#if defined(__GNUC__)
# include <cxxabi.h>   // for abi::__cxa_demangle
#endif
# include <typeinfo>   // for typeid
# include <cstdlib>    // for std::free
#endif
//-------------------
#include <complex>
#include <vector>
#include <list>
#include <deque>
#include <string>
#include <utility>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>
#include <tuple>
//---Seiriakos library------------------------------------------------------------------------------
namespace Seiriakos
{
  namespace Version
  {
    constexpr unsigned long MAJOR  = 000;
    constexpr unsigned long MINOR  = 001;
    constexpr unsigned long PATCH  = 000;
    constexpr unsigned long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }

  enum class Status : char;

  // serialize "thing"
  template<typename T> inline
  auto serialize(const T& thing) noexcept -> std::vector<uint8_t>;

  // deserialize data into "thing"
  template<typename T> inline
  auto deserialize(T& thing, const uint8_t data[], const size_t size) noexcept -> Status;

  // abstract class to add serialization capabilities
  class Serializable;

  // macro to easily implement serialization/deserialization from a list of variables
# define SEIRIAKOS_SEQUENCE(...)

  // print bytes from memory
  inline
  void print_bytes(const uint8_t data[], const size_t size, const bool print_header = true);

  namespace Global
  {
    std::ostream out{std::cout.rdbuf()}; // output ostream
    std::ostream err{std::cerr.rdbuf()}; // error ostream
    std::ostream log{std::clog.rdbuf()}; // logging ostream
  }
//----------------------------------------------------------------------------------------------------------------------
  namespace _backend
  {
# if defined(__STDCPP_THREADS__)
    static thread_local std::vector<uint8_t> _buffer;
    static thread_local size_t               _front_of_buffer;
    static thread_local Status               _status;
# else
    static std::vector<uint8_t> _buffer;
    static size_t               _front_of_buffer;
    static Status               _status;
#   endif

//     void _error(const char* message, Status ec) noexcept
//     {
// #   if defined(__STDCPP_THREADS__)
//       static std::mutex mtx;
//       std::lock_guard<std::mutex> lock{mtx};
// #   endif
//       _status = ec;
//       Global::err << "error: Seiriakos: " << message << std::endl;
//     }

# if defined(SEIRIAKOS_LOGGING)
    class _indentlog
    {
    public:
      _indentlog(std::string text) noexcept
      {
#     if defined(__STDCPP_THREADS__)
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock{mtx};
#     endif

        for (unsigned k = indentation++; k; --k)
        {
          Global::log << "  ";
        }

        Global::log << text << std::endl;
      }

      ~_indentlog() noexcept { --indentation; }
    private:
#   if defined(__STDCPP_THREADS__)
      static std::atomic_uint indentation;
#   else
      static unsigned indentation;
#   endif
    };

# if defined(__STDCPP_THREADS__)
    std::atomic_uint _indentlog::indentation;
# else
    unsigned _indentlog::indentation;
# endif

    template<typename T> inline
    std::string _underlying_name()
    {
      if (std::is_integral<T>::value)
      {
        return (std::is_unsigned<T>::value ? "uint" : "int") + std::to_string(sizeof(T) * 8);
      }
      else if (std::is_floating_point<T>::value)
      {
        return "float" + std::to_string(sizeof(T) * 8);
      }
      else
      {
        return abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
      }
    }

#   define SEIRIAKOS_ILOG(text) _backend::_indentlog ilog{text}
#   define SEIRIAKOS_LOG(text)  _backend::_indentlog{text}
# else
#   define SEIRIAKOS_ILOG(text) void(0)
#   define SEIRIAKOS_LOG(text) void(0)
# endif

    template<typename T>
    using _if_not_Serializable = typename std::enable_if<not std::is_base_of<Serializable, T>::value>::type;

    inline
    void _serialization_implementation(const Serializable& data);
    inline
    void _deserialization_implementation(Serializable& data);

    template<typename T, typename = _if_not_Serializable<T>> inline
    void _serialization_implementation(const T& data);
    template<typename T, typename = _if_not_Serializable<T>> inline
    void _deserialization_implementation(T& data);

    inline
    void size_t_serialization_implementation(size_t size);
    inline
    void size_t_deserialization_implementation(size_t& size);

    template<typename T> inline
    void _serialization_implementation(const std::complex<T>& complex);
    template<typename T> inline
    void _deserialization_implementation(std::complex<T>& complex);

    template<typename T> inline
    void _serialization_implementation(const std::basic_string<T>& string);
    template<typename T> inline
    void _deserialization_implementation(std::basic_string<T>& string);

    template<typename T> inline
    void _serialization_implementation(const std::vector<T>& vector);
    template<typename T> inline
    void _deserialization_implementation(std::vector<T>& vector);

    template<typename T> inline
    void _serialization_implementation(const std::list<T>& list);
    template<typename T> inline
    void _deserialization_implementation(std::list<T>& list);

    template<typename T> inline
    void _serialization_implementation(const std::deque<T>& deque);
    template<typename T> inline
    void _deserialization_implementation(std::deque<T>& deque);

    template<typename T1, typename T2> inline
    void _serialization_implementation(const std::pair<T1, T2>& pair);
    template<typename T1, typename T2> inline
    void _deserialization_implementation(std::pair<T1, T2>& pair);

    template<typename T1, typename T2> inline
    void _serialization_implementation(const std::unordered_map<T1, T2>& unordered_map);
    template<typename T1, typename T2> inline
    void _deserialization_implementation(std::unordered_map<T1, T2>& unordered_map);

    template<typename T1, typename T2> inline
    void _serialization_implementation(const std::unordered_multimap<T1, T2>& unordered_multimap);
    template<typename T1, typename T2> inline
    void _deserialization_implementation(std::unordered_multimap<T1, T2>& unordered_multimap);

    template<typename T1, typename T2> inline
    void _serialization_implementation(const std::map<T1, T2>& map);
    template<typename T1, typename T2> inline
    void _deserialization_implementation(std::map<T1, T2>& map);

    template<typename T1, typename T2> inline
    void _serialization_implementation(const std::multimap<T1, T2>& multimap);
    template<typename T1, typename T2> inline
    void _deserialization_implementation(std::multimap<T1, T2>& multimap);

    template<typename T> inline
    void _serialization_implementation(const std::unordered_set<T>& unordered_set);
    template<typename T> inline
    void _deserialization_implementation(std::unordered_set<T>& unordered_set);

    template<typename T> inline
    void _serialization_implementation(const std::unordered_multiset<T>& unordered_multiset);
    template<typename T> inline
    void _deserialization_implementation(std::unordered_multiset<T>& unordered_multiset);

    template<typename T> inline
    void _serialization_implementation(const std::set<T>& set);
    template<typename T> inline
    void _deserialization_implementation(std::set<T>& set);

    template<typename T> inline
    void _serialization_implementation(const std::multiset<T>& multiset);
    template<typename T> inline
    void _deserialization_implementation(std::multiset<T>& multiset);

    template<typename... T> inline
    void _serialization_implementation(const std::tuple<T...>& tuple);
    template<typename... T> inline
    void _deserialization_implementation(std::multiset<T...>& tuple);

# if defined(__GNUC__) and (__GNUC__ >= 9)
#   define SEIRIAKOS_UNLIKELY [[unlikely]]
# elif defined(__clang__) and (__clang_major__ >= 9)
#   define SEIRIAKOS_UNLIKELY [[unlikely]]
# else
#   define SEIRIAKOS_UNLIKELY
# endif
  }
//----------------------------------------------------------------------------------------------------------------------
  enum class Status : char
  {
    ALL_GOOD,
    MISSING_BYTES,
    EMPTY_BUFFER,
    SEQUENCE_MISMATCH,
    NOT_IMPLEMENTED_YET
  };

  template<typename T>
  auto serialize(const T& thing) noexcept -> std::vector<uint8_t>
  {
    SEIRIAKOS_LOG("----serialization summary:");
    _backend::_buffer.clear();
    _backend::_serialization_implementation(thing);
    SEIRIAKOS_LOG("----------------------------");
    return _backend::_buffer;
  }

  template<typename T>
  auto deserialize(T& thing, const uint8_t data[], const size_t size) noexcept -> Status
  {
    SEIRIAKOS_LOG("----deserialization summary:");
    _backend::_buffer.assign(data, data + size);
    _backend::_front_of_buffer = 0;
    _backend::_status      = Status::ALL_GOOD;
    _backend::_deserialization_implementation(thing);

    if (_backend::_front_of_buffer != _backend::_buffer.size())
    {
      // _backend::_error("buffer is not empty, serialization/deserialization sequence mismatch", Status::SEQUENCE_MISMATCH);
      _backend::_status = Status::SEQUENCE_MISMATCH;
    }

    SEIRIAKOS_LOG("----------------------------");

    return _backend::_status;
  }

  class Serializable
  {
  public:
    std::vector<uint8_t> serialize() const noexcept
    {
      return Seiriakos::serialize(*this);
    }

    Status deserialize(const uint8_t data[], const size_t size) noexcept
    {
      return Seiriakos::deserialize(*this, data, size);
    }
  protected:
    // serialization/deserialization sequence (provided by the inheriting class)
    virtual void serialization_sequence()   const noexcept = 0;
    virtual void deserialization_sequence()       noexcept = 0;

    // recursive calls to appropriate _serialization_implementation overloads
    template<typename T, typename... Tn> inline
    void serialization(const T& data, const Tn&... data_n) const noexcept
    {
      _backend::_serialization_implementation(data);
      serialization(data_n...);
    }
    void serialization() const noexcept {};

    // recursive calls to appropriate _deserialization_implementation overloads
    template<typename T, typename... Tn> inline
    void deserialization(T& data, Tn&... data_n) noexcept
    {
      _backend::_deserialization_implementation(data);
      deserialization(data_n...);
    }
    void deserialization() noexcept {};
    
  friend void _backend::_serialization_implementation(const Serializable& data);
  friend void _backend::_deserialization_implementation(Serializable& data);
  };

# undef  SEIRIAKOS_SEQUENCE
# define SEIRIAKOS_SEQUENCE(...)                            \
    private:                                                \
      void serialization_sequence() const noexcept override \
      {                                                     \
        serialization(__VA_ARGS__);                         \
      }                                                     \
      void deserialization_sequence() noexcept override     \
      {                                                     \
        deserialization(__VA_ARGS__);                       \
      }

  void print_bytes(const uint8_t data[], const size_t size, const bool print_header)
  {
    if (print_header)
    {
      Global::out << "bytes[" << std::dec << size << "]: ";
    }

    Global::out << std::hex << std::setfill('0');
    for (size_t k = 0; k < size; ++k)
    {
      Global::out << std::setw(2) << std::uppercase << unsigned(data[k] & 0xFF) << ' ';
    }

    Global::out << std::endl;
  }
//----------------------------------------------------------------------------------------------------------------------
  namespace _backend
  {
    void _serialization_implementation(const Serializable& data)
    {
      SEIRIAKOS_ILOG("Serializable");
      
      data.serialization_sequence(); // serialize data via its specialized implementation
    }

    void _deserialization_implementation(Serializable& data)
    {
      SEIRIAKOS_ILOG("Serializable");

      data.deserialization_sequence(); // serialize data via its specialized implementation
    }

    template<typename T, typename>
    void _serialization_implementation(const T& data)
    {
      SEIRIAKOS_ILOG(_underlying_name<T>());

      // add data's bytes one by one to the buffer
      for (size_t k = 0; k < sizeof(T); ++k)
      {
        _buffer.push_back(reinterpret_cast<const uint8_t*>(&data)[k]);
      }
    }

    template<typename T, typename>
    void _deserialization_implementation(T& data)
    {
      SEIRIAKOS_ILOG(_underlying_name<T>());

      if (_front_of_buffer >= _buffer.size()) SEIRIAKOS_UNLIKELY
      {
        // _error("could not deserialize data, buffer is empty", Status::EMPTY_BUFFER);
        _backend::_status = Status::EMPTY_BUFFER;
        return;
      }

      if ((_buffer.size() - _front_of_buffer) < sizeof(T)) SEIRIAKOS_UNLIKELY
      {
        // _error("could not deserialize data, not enough bytes in buffer", Status::MISSING_BYTES);
        _backend::_status = Status::MISSING_BYTES;
        return;
      }

      // set data's bytes one by one from the front of the buffer
      for (size_t k = 0; k < sizeof(T); ++k)
      {
        reinterpret_cast<uint8_t*>(&data)[k] = _buffer[_front_of_buffer++];
      }
    }

    void size_t_serialization_implementation(size_t size)
    {
      SEIRIAKOS_ILOG("size_t");

      // compute minimum amount of bytes needed to serialize
      uint8_t bytes_used = 1;
      for (size_t k = size; k >>= 8; ++bytes_used)
      {}

      _buffer.push_back(bytes_used);

      for (size_t k = 0; bytes_used--; k += 8)
      {
        _buffer.push_back(static_cast<uint8_t>((size >> k) & 0xFF));
      }
    }

    void size_t_deserialization_implementation(size_t& size)
    {
      SEIRIAKOS_ILOG("size_t");

      if (_front_of_buffer >= _buffer.size()) SEIRIAKOS_UNLIKELY
      {
        // _error("could not deserialize data, buffer is empty", Status::EMPTY_BUFFER);
        _backend::_status = Status::EMPTY_BUFFER;
        return;
      }

      uint8_t bytes_used = _buffer[_front_of_buffer++];

      if ((_buffer.size() - _front_of_buffer) < bytes_used) SEIRIAKOS_UNLIKELY
      {
        // _error("could not deserialize data, not enough bytes in buffer", Status::MISSING_BYTES);
        _backend::_status = Status::MISSING_BYTES;
        return;
      }

      size = 0;
      for (size_t k = 0; bytes_used--; k += 8)
      {
        size |= (_buffer[_front_of_buffer++] << k);
      }
    }

    template<typename T> inline
    void _serialization_implementation(const std::complex<T>& complex)
    {
      SEIRIAKOS_ILOG("std::complex");

      _serialization_implementation(complex.real);
      _serialization_implementation(complex.imag);
    }

    template<typename T> inline
    void _deserialization_implementation(std::complex<T>& complex)
    {
      SEIRIAKOS_ILOG("std::complex");
      
      _deserialization_implementation(complex.real);
      _deserialization_implementation(complex.imag);
    }

    template<typename T>
    void _serialization_implementation(const std::basic_string<T>& string)
    {
      SEIRIAKOS_ILOG("std::string");

      size_t_serialization_implementation(string.size());

      for (T character : string)
      {
        _serialization_implementation(character);
      }
    }

    template<typename T>
    void _deserialization_implementation(std::basic_string<T>& string)
    {
      SEIRIAKOS_ILOG("std::string");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      string.clear();

      T character = '\n';
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(character);
        string += character;
      }
    }

    template<typename T>
    void _serialization_implementation(const std::vector<T>& vector)
    {
      SEIRIAKOS_ILOG("std::vector");

      size_t_serialization_implementation(vector.size());

      for (const auto& value : vector)
      {
        _serialization_implementation(value);
      }
    }

    template<typename T>
    void _deserialization_implementation(std::vector<T>& vector)
    {
      SEIRIAKOS_ILOG("std::vector");

      vector.clear();

      size_t size = 0;
      size_t_deserialization_implementation(size);

      vector.reserve(size);

      T value;
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(value);
        vector.push_back(value);
      }
    }

    template<typename T>
    void _serialization_implementation(const std::list<T>& list)
    {
      SEIRIAKOS_ILOG("std::list");

      size_t_serialization_implementation(list.size());

      for (const auto& value : list)
      {
        _serialization_implementation(value);
      }
    }

    template<typename T>
    void _deserialization_implementation(std::list<T>& list)
    {
      SEIRIAKOS_ILOG("std::list");

      list.clear();

      size_t size = 0;
      size_t_deserialization_implementation(size);

      T value;
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(value);
        list.push_back(value);
      }
    }

    template<typename T>
    void _serialization_implementation(const std::deque<T>& deque)
    {
      SEIRIAKOS_ILOG("std::deque");

      size_t_serialization_implementation(deque.size());

      for (const auto& value : deque)
      {
        _serialization_implementation(value);
      }
    }

    template<typename T>
    void _deserialization_implementation(std::deque<T>& deque)
    {
      SEIRIAKOS_ILOG("std::deque");

      deque.clear();

      size_t size = 0;
      size_t_deserialization_implementation(size);

      T value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(value);
        deque.push_back(value);
      }
    }

    template<typename T1, typename T2>
    void _serialization_implementation(const std::pair<T1, T2>& pair)
    {
      SEIRIAKOS_ILOG("std::pair");

      _serialization_implementation(pair.first);
      _serialization_implementation(pair.second);
    }

    template<typename T1, typename T2>
    void _deserialization_implementation(std::pair<T1, T2>& pair)
    {
      SEIRIAKOS_ILOG("std::pair");

      _deserialization_implementation(pair.first);
      _deserialization_implementation(pair.second);
    }

    template<typename T1, typename T2>
    void _serialization_implementation(const std::unordered_map<T1, T2>& unordered_map)
    {
      SEIRIAKOS_ILOG("std::unordered_map");

      size_t_serialization_implementation(unordered_map.size());

      for (const auto& key_value : unordered_map)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    void _deserialization_implementation(std::unordered_map<T1, T2>& unordered_map)
    {
      SEIRIAKOS_ILOG("std::unordered_map");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      unordered_map.clear();

      std::pair<T1, T2> key_value;
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key_value);
        unordered_map.insert(key_value);
      }
    }

    template<typename T1, typename T2>
    void _serialization_implementation(const std::unordered_multimap<T1, T2>& unordered_multimap)
    {
      SEIRIAKOS_ILOG("std::unordered_multimap");
      
      size_t_serialization_implementation(unordered_multimap.size());

      for (const auto& key_value : unordered_multimap)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    void _deserialization_implementation(std::unordered_multimap<T1, T2>& unordered_multimap)
    {
      SEIRIAKOS_ILOG("std::unordered_multimap");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      unordered_multimap.clear();

      std::pair<T1, T2> key_value;
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key_value);
        unordered_multimap.insert(key_value);
      }
    }

    template<typename T1, typename T2>
    void _serialization_implementation(const std::map<T1, T2>& map)
    {
      SEIRIAKOS_ILOG("std::map");
      
      size_t_serialization_implementation(map.size());

      for (const auto& key_value : map)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    void _deserialization_implementation(std::map<T1, T2>& map)
    {
      SEIRIAKOS_ILOG("std::map");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      map.clear();

      std::pair<T1, T2> key_value;
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key_value);
        map.insert(key_value);
      }
    }

    template<typename T1, typename T2>
    void _serialization_implementation(const std::multimap<T1, T2>& multimap)
    {
      SEIRIAKOS_ILOG("std::multimap");
      
      size_t_serialization_implementation(multimap.size());

      for (const auto& key_value : multimap)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    void _deserialization_implementation(std::multimap<T1, T2>& multimap)
    {
      SEIRIAKOS_ILOG("std::multimap");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      multimap.clear();

      std::pair<T1, T2> key_value;
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key_value);
        multimap.insert(key_value);
      }
    }

    template<typename T>
    void _serialization_implementation(const std::unordered_set<T>& unordered_set)
    {
      SEIRIAKOS_ILOG("std::unordered_set");

      size_t_serialization_implementation(unordered_set.size());

      for (const auto& key : unordered_set)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    void _deserialization_implementation(std::unordered_set<T>& unordered_set)
    {
      SEIRIAKOS_ILOG("std::unordered_set");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      unordered_set.clear();

      T key;
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key);
        unordered_set.insert(key);
      }
    }

    template<typename T>
    void _serialization_implementation(const std::unordered_multiset<T>& unordered_multiset)
    {
      SEIRIAKOS_ILOG("std::unordered_multiset");
      
      size_t_serialization_implementation(unordered_multiset.size());

      for (const auto& key : unordered_multiset)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    void _deserialization_implementation(std::unordered_multiset<T>& unordered_multiset)
    {
      SEIRIAKOS_ILOG("std::unordered_multiset");
      
      size_t size = 0;
      size_t_deserialization_implementation(size);

      unordered_multiset.clear();

      T key;
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key);
        unordered_multiset.insert(key);
      }
    }

    template<typename T>
    void _serialization_implementation(const std::set<T>& set)
    {
      SEIRIAKOS_ILOG("std::set");
      
      size_t_serialization_implementation(set.size());

      for (const auto& key : set)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    void _deserialization_implementation(std::set<T>& set)
    {
      SEIRIAKOS_ILOG("std::set");
      
      size_t size = 0;
      size_t_deserialization_implementation(size);

      set.clear();

      T key;
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key);
        set.insert(key);
      }
    }

    template<typename T>
    void _serialization_implementation(const std::multiset<T>& multiset)
    {
      SEIRIAKOS_ILOG("std::multiset");
      
      size_t_serialization_implementation(multiset.size());

      for (const auto& key : multiset)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    void _deserialization_implementation(std::multiset<T>& multiset)
    {
      SEIRIAKOS_ILOG("std::multiset");
      
      size_t size = 0;
      size_t_deserialization_implementation(size);

      multiset.clear();

      T key;
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key);
        multiset.insert(key);
      }
    }

    template<typename... T>
    void _serialization_implementation(const std::tuple<T...>&)
    {
      // _error("tuples are not implemented yet", Status::NOT_IMPLEMENTED_YET);
      _backend::_status = Status::NOT_IMPLEMENTED_YET;
    }

    template<typename... T>
    void _deserialization_implementation(std::tuple<T...>&)
    {
      // _error("tuples are not implemented yet", Status::NOT_IMPLEMENTED_YET);
      _backend::_status = Status::NOT_IMPLEMENTED_YET;
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  #undef SEIRIAKOS_ILOG
}
#endif