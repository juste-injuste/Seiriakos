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
#include <sstream>     // for std::stringstream
#include <iomanip>     // for std::setw, std::setfill, std::hex
#include <ios>         // for std::uppercase
#include <cstring>     // for std::memcpy
#if defined(__STDCPP_THREADS__) and not defined(SEIRIAKOS_NOT_THREADSAFE)
# define  SEIRIAKOS_THREADSAFE
# include <atomic>     // for std::atomic
# include <mutex>      // for std::mutex, std::lock_guard
#endif 
#if defined(SEIRIAKOS_LOGGING)
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
  enum class Info : uint8_t
  {
    ALL_GOOD            = 0,
    MISSING_BYTES       = 1,
    EMPTY_BUFFER        = 2,
    SEQUENCE_MISMATCH   = 3,
    NOT_IMPLEMENTED_YET = 4
  };

  // serialize "thing"
  template<typename T> inline
  auto serialize(const T& thing) noexcept -> std::vector<uint8_t>;

  // deserialize data into "thing"
  template<typename T> inline
  Info deserialize(T& thing, const uint8_t data[], const size_t size) noexcept;

  // abstract class to add serialization capabilities
  class Serializable;

  // macro to facilitate serialization/deserialization member function implementations
# define SEIRIAKOS_SEQUENCE(...)

  // print bytes from memory
  inline
  const char* bytes_as_cstring(const uint8_t data[], const size_t size);

  namespace Global
  {
    std::ostream out{std::cout.rdbuf()}; // output ostream
    std::ostream err{std::cerr.rdbuf()}; // error ostream
    std::ostream log{std::clog.rdbuf()}; // logging ostream
  }

  namespace Version
  {
    constexpr unsigned long MAJOR  = 000;
    constexpr unsigned long MINOR  = 001;
    constexpr unsigned long PATCH  = 000;
    constexpr unsigned long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
//----------------------------------------------------------------------------------------------------------------------
  namespace _backend
  {
# if defined(__clang__) and (__clang_major__ >= 12)
#   define SEIRIAKOS_HOT  [[likely]]
#   define SEIRIAKOS_COLD [[unlikely]]
# elif defined(__GNUC__) and (__GNUC__ >= 9)
#   define SEIRIAKOS_HOT  [[likely]]
#   define SEIRIAKOS_COLD [[unlikely]]
# else
#   define SEIRIAKOS_COLD
# endif

# if defined(SEIRIAKOS_THREADSAFE)
#   define SEIRIAKOS_THREADLOCAL     thread_local
#   define SEIRIAKOS_ATOMIC(T)       std::atomic<T>
#   define SEIRIAKOS_MAKE_MUTEX(...) static std::mutex __VA_ARGS__
#   define SEIRIAKOS_LOCK(MUTEX)     std::lock_guard<decltype(MUTEX)> _lock{MUTEX}
# else
#   define SEIRIAKOS_THREADLOCAL
#   define SEIRIAKOS_ATOMIC(T)       T
#   define SEIRIAKOS_MAKE_MUTEX(...)
#   define SEIRIAKOS_LOCK(MUTEX)     void(0)
# endif

    static SEIRIAKOS_THREADLOCAL std::vector<uint8_t> _buffer;
    static SEIRIAKOS_THREADLOCAL size_t _front_of_buffer;
    static SEIRIAKOS_THREADLOCAL Info _info;

# if defined(SEIRIAKOS_LOGGING)
    class _indentlog
    {
    public:
      _indentlog(std::string text) noexcept
      {
        SEIRIAKOS_MAKE_MUTEX(mtx);
        SEIRIAKOS_LOCK(mtx);

        for (unsigned k = indentation++; k; --k)
        {
          Global::log << "  ";
        }

        Global::log << text << std::endl;
      }

      ~_indentlog() noexcept { --indentation; }
    private:
      static SEIRIAKOS_ATOMIC(unsigned) indentation;
    };

    SEIRIAKOS_ATOMIC(unsigned) _indentlog::indentation;

    template<typename T>
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
#   if defined(__clang__) or defined(__GNUC__)
      else
      {
        return abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
      }
#   else
      else
      {
        return typeid(T).name();
      }
#   endif
    }

#   define SEIRIAKOS_ILOG(text) _backend::_indentlog ilog{text}
#   define SEIRIAKOS_LOG(text)  _backend::_indentlog{text}
# else
#   define SEIRIAKOS_ILOG(text) void(0)
#   define SEIRIAKOS_LOG(text)  void(0)
# endif

    template<typename T>
    using _if_not_Serializable = typename std::enable_if<not std::is_base_of<Serializable, T>::value>::type;
   
    void _serialization_implementation(const Serializable& data);
    void _deserialization_implementation(Serializable& data);

    template<typename T, typename = _if_not_Serializable<T>>
    void _serialization_implementation(const T& data, size_t N = 1)
    {
      SEIRIAKOS_ILOG(_underlying_name<T>() + (N > 1 ? " x" + std::to_string(N) : ""));

      const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(&data);
      _buffer.insert(_buffer.end(), data_ptr, data_ptr + sizeof(T) * N);
    }

    template<typename T, typename = _if_not_Serializable<T>>
    void _deserialization_implementation(T& data, size_t N = 1)
    {
      SEIRIAKOS_ILOG(_underlying_name<T>() + (N > 1 ? " x" + std::to_string(N) : ""));

      if (_front_of_buffer >= _buffer.size()) SEIRIAKOS_COLD
      {
        _info = Info::EMPTY_BUFFER;
        return;
      }

      if ((_buffer.size() - _front_of_buffer) < (sizeof(T) * N)) SEIRIAKOS_COLD
      {
        _info = Info::MISSING_BYTES;
        return;
      }

      // set data's bytes one by one from the front of the buffer
      uint8_t* data_ptr    = reinterpret_cast<uint8_t*>(&data);
      uint8_t* _buffer_ptr = _buffer.data() + _front_of_buffer;
      std::memcpy(data_ptr, _buffer_ptr, sizeof(T) * N);

      _front_of_buffer += sizeof(T) * N;
    }

    void size_t_serialization_implementation(size_t size)
    {
      SEIRIAKOS_ILOG("size_t");

      // compute minimum amount of bytes needed to serialize
      uint8_t bytes_used = 1;
      for (size_t k = size; k >>= 8; ++bytes_used) {}

      _buffer.push_back(bytes_used);

      for (size_t k = 0; bytes_used--; k += 8)
      {
        _buffer.push_back(static_cast<uint8_t>((size >> k) & 0xFF));
      }
    }

    void size_t_deserialization_implementation(size_t& size)
    {
      SEIRIAKOS_ILOG("size_t");

      if (_front_of_buffer >= _buffer.size()) SEIRIAKOS_COLD
      {
        _info = Info::EMPTY_BUFFER;
        return;
      }

      uint8_t bytes_used = _buffer[_front_of_buffer++];

      if ((_buffer.size() - _front_of_buffer) < bytes_used) SEIRIAKOS_COLD
      {
        _info = Info::MISSING_BYTES;
        return;
      }

      size = 0;
      for (size_t k = 0; bytes_used--; k += 8)
      {
        size |= (_buffer[_front_of_buffer++] << k);
      }
    }

    template<typename T>
    void _serialization_implementation(const std::complex<T>& complex);
    template<typename T>
    void _deserialization_implementation(std::complex<T>& complex);

    template<typename T>
    void _serialization_implementation(const std::basic_string<T>& string);
    template<typename T>
    void _deserialization_implementation(std::basic_string<T>& string);

    template<typename T, size_t N>
    void _serialization_implementation(const std::array<T, N>& array);
    template<typename T, size_t N>
    void _deserialization_implementation(std::array<T, N>& array);

    template<typename T>
    void _serialization_implementation(const std::vector<T>& vector);
    template<typename T>
    void _deserialization_implementation(std::vector<T>& vector);

    template<typename T>
    void _serialization_implementation(const std::list<T>& list);
    template<typename T>
    void _deserialization_implementation(std::list<T>& list);

    template<typename T>
    void _serialization_implementation(const std::deque<T>& deque);
    template<typename T>
    void _deserialization_implementation(std::deque<T>& deque);

    template<typename T1, typename T2>
    void _serialization_implementation(const std::pair<T1, T2>& pair);
    template<typename T1, typename T2>
    void _deserialization_implementation(std::pair<T1, T2>& pair);

    template<typename T1, typename T2>
    void _serialization_implementation(const std::unordered_map<T1, T2>& unordered_map);
    template<typename T1, typename T2>
    void _deserialization_implementation(std::unordered_map<T1, T2>& unordered_map);

    template<typename T1, typename T2>
    void _serialization_implementation(const std::unordered_multimap<T1, T2>& unordered_multimap);
    template<typename T1, typename T2>
    void _deserialization_implementation(std::unordered_multimap<T1, T2>& unordered_multimap);

    template<typename T1, typename T2>
    void _serialization_implementation(const std::map<T1, T2>& map);
    template<typename T1, typename T2>
    void _deserialization_implementation(std::map<T1, T2>& map);

    template<typename T1, typename T2>
    void _serialization_implementation(const std::multimap<T1, T2>& multimap);
    template<typename T1, typename T2>
    void _deserialization_implementation(std::multimap<T1, T2>& multimap);

    template<typename T>
    void _serialization_implementation(const std::unordered_set<T>& unordered_set);
    template<typename T>
    void _deserialization_implementation(std::unordered_set<T>& unordered_set);

    template<typename T>
    void _serialization_implementation(const std::unordered_multiset<T>& unordered_multiset);
    template<typename T>
    void _deserialization_implementation(std::unordered_multiset<T>& unordered_multiset);

    template<typename T>
    void _serialization_implementation(const std::set<T>& set);
    template<typename T>
    void _deserialization_implementation(std::set<T>& set);

    template<typename T>
    void _serialization_implementation(const std::multiset<T>& multiset);
    template<typename T>
    void _deserialization_implementation(std::multiset<T>& multiset);

    template<typename... T>
    void _serialization_implementation(const std::tuple<T...>& tuple);
    template<typename... T>
    void _deserialization_implementation(std::tuple<T...>& tuple);
  }
//----------------------------------------------------------------------------------------------------------------------
  class Serializable
  {
  public:
    inline
    std::vector<uint8_t> serialize() const noexcept;

    inline
    Info deserialize(const uint8_t data[], const size_t size) noexcept;
  protected:
    // serialization/deserialization sequence (provided by the inheriting class)
    virtual void serialization_sequence() const noexcept = 0;
    virtual void deserialization_sequence()     noexcept = 0;

    // recursive calls to appropriate _serialization_implementation overloads
    template<typename T, typename... T_> inline
    void serialization(const T& data, const T_&... remaining) const noexcept;
    void serialization()                                      const noexcept {};

    // recursive calls to appropriate _deserialization_implementation overloads
    template<typename T, typename... T_> inline
    void deserialization(T& data, T_&... remaining) noexcept;
    void deserialization()                          noexcept {};
    
  friend void _backend::_serialization_implementation(const Serializable& data);
  friend void _backend::_deserialization_implementation(Serializable& data);
  };
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

    template<typename T>
    void _serialization_implementation(const std::complex<T>& complex)
    {
      SEIRIAKOS_ILOG("std::complex");

      _serialization_implementation(complex.real);
      _serialization_implementation(complex.imag);
    }

    template<typename T>
    void _deserialization_implementation(std::complex<T>& complex)
    {
      SEIRIAKOS_ILOG("std::complex");
      
      _deserialization_implementation(complex.real);
      _deserialization_implementation(complex.imag);
    }

    template<typename T>
    void _serialization_implementation(const std::basic_string<T>& string)
    {
      SEIRIAKOS_ILOG("std::basic_string");

      size_t_serialization_implementation(string.size());

      if (std::is_fundamental<T>::value) SEIRIAKOS_HOT
      {
        _serialization_implementation(string[0], string.size());
      }
      else
      {
        for (T character : string)
        {
          _serialization_implementation(character);
        }
      }
    }

    template<typename T>
    void _deserialization_implementation(std::basic_string<T>& string)
    {
      SEIRIAKOS_ILOG("std::basic_string");

      size_t size = {};
      size_t_deserialization_implementation(size);

      string.resize(size);

      if (std::is_fundamental<T>::value) SEIRIAKOS_HOT
      {
        _deserialization_implementation(string[0], size);
      }
      else
      {
        for (auto& character : string)
        {
          _deserialization_implementation(character);
        }
      }
    }

    template<typename T, size_t N>
    void _serialization_implementation(const std::array<T, N>& array)
    {
      SEIRIAKOS_ILOG("std::array");

      if (std::is_fundamental<T>::value) SEIRIAKOS_HOT
      {
        _serialization_implementation(array[0], N);
      }
      else
      {
        for (const auto& value : array)
        {
          _serialization_implementation(value);
        }
      }
    }

    template<typename T, size_t N>
    void _deserialization_implementation(std::array<T, N>& array)
    {
      SEIRIAKOS_ILOG("std::array");

      if (std::is_fundamental<T>::value) SEIRIAKOS_HOT
      {
        _deserialization_implementation(array[0], N);
      }
      else
      {
        for (auto& value : array)
        {
          _deserialization_implementation(value);
        }
      }
    }

    template<typename T>
    void _serialization_implementation(const std::vector<T>& vector)
    {
      SEIRIAKOS_ILOG("std::vector");

      size_t_serialization_implementation(vector.size());

      if (std::is_fundamental<T>::value) SEIRIAKOS_HOT
      {
        _serialization_implementation(vector[0], vector.size());
      }
      else
      {
        for (const auto& value : vector)
        {
          _serialization_implementation(value);
        }
      }
    }

    template<typename T>
    void _deserialization_implementation(std::vector<T>& vector)
    {
      SEIRIAKOS_ILOG("std::vector");

      size_t size = {};
      size_t_deserialization_implementation(size);

      vector.resize(size);

      if (std::is_fundamental<T>::value) SEIRIAKOS_HOT
      {
        _deserialization_implementation(vector[0], size);
      }
      else
      {
        for (auto& value : vector)
        {
          _deserialization_implementation(value);
        }
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

      size_t size = {};
      size_t_deserialization_implementation(size);

      list.resize(size);
      for (auto& value : list)
      {
        _deserialization_implementation(value);
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

      size_t size = {};
      size_t_deserialization_implementation(size);
      
      deque.resize(size);
      for (auto& value : deque)
      {
        _deserialization_implementation(value);
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

      size_t size = {};
      size_t_deserialization_implementation(size);

      unordered_map.clear();
      unordered_map.reserve(size);

      std::pair<T1, T2> key_value = {};
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

      size_t size = {};
      size_t_deserialization_implementation(size);

      unordered_multimap.clear();
      unordered_multimap.reserve(size);

      std::pair<T1, T2> key_value = {};
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

      size_t size = {};
      size_t_deserialization_implementation(size);

      map.clear();

      std::pair<T1, T2> key_value = {};
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

      size_t size = {};
      size_t_deserialization_implementation(size);

      multimap.clear();

      std::pair<T1, T2> key_value = {};
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

      size_t size = {};
      size_t_deserialization_implementation(size);

      unordered_set.clear();
      unordered_set.reserve(size);

      T key = {};
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
      
      size_t size = {};
      size_t_deserialization_implementation(size);

      unordered_multiset.clear();
      unordered_multiset.reserve(size);

      T key = {};
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
      
      size_t size = {};
      size_t_deserialization_implementation(size);

      set.clear();

      T key = {};
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
      
      size_t size = {};
      size_t_deserialization_implementation(size);

      multiset.clear();

      T key = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key);
        multiset.insert(key);
      }
    }

    template<size_t N, typename... T>
    struct _tuple_serialization
    {
      inline
      static void implementation(const std::tuple<T...>& tuple);
    };

    template<typename... T>
    struct _tuple_serialization<0, T...>
    {
      static void implementation(const std::tuple<T...>&) {}
    };

    template<size_t N, typename... T>
    void _tuple_serialization<N, T...>::implementation(const std::tuple<T...>& tuple)
    {
      _serialization_implementation(std::get<sizeof...(T) - N>(tuple));
      _tuple_serialization<N-1, T...>::implementation(tuple);
    }

    template<typename... T>
    void _serialization_implementation(const std::tuple<T...>& tuple)
    {
      SEIRIAKOS_ILOG("std::tuple");

      _tuple_serialization<sizeof...(T), T...>::implementation(tuple);
    }

    template<size_t N, typename... T>
    struct _tuple_deserialization
    {
      inline
      static void implementation(std::tuple<T...>& tuple);
    };

    template<typename... T>
    struct _tuple_deserialization<0, T...>
    {
      static void implementation(std::tuple<T...>&) {}
    };

    template<size_t N, typename... T>
    void _tuple_deserialization<N, T...>::implementation(std::tuple<T...>& tuple)
    {
      _deserialization_implementation(std::get<sizeof...(T) - N>(tuple));
      _tuple_deserialization<N-1, T...>::implementation(tuple);
    }

    template<typename... T>
    void _deserialization_implementation(std::tuple<T...>& tuple)
    {
      SEIRIAKOS_ILOG("std::tuple");

      _tuple_deserialization<sizeof...(T), T...>::implementation(tuple);
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  template<typename T>
  auto serialize(const T& thing) noexcept -> std::vector<uint8_t>
  {
    SEIRIAKOS_LOG("----serialization summary:");
    _backend::_buffer.clear();
    _backend::_buffer.reserve(sizeof(thing));
    _backend::_serialization_implementation(thing);
    SEIRIAKOS_LOG("----------------------------");
    return _backend::_buffer;
  }

  template<typename T>
  Info deserialize(T& thing, const uint8_t data[], const size_t size) noexcept
  {
    SEIRIAKOS_LOG("----deserialization summary:");
    _backend::_buffer.assign(data, data + size);
    _backend::_front_of_buffer = 0;
    _backend::_info = Info::ALL_GOOD;
    _backend::_deserialization_implementation(thing);

    if (_backend::_front_of_buffer != _backend::_buffer.size())
    {
      _backend::_info = Info::SEQUENCE_MISMATCH;
    }

    SEIRIAKOS_LOG("----------------------------");

    return _backend::_info;
  }
    
  std::vector<uint8_t> Serializable::serialize() const noexcept
  {
    return Seiriakos::serialize(*this);
  }

  Info Serializable::deserialize(const uint8_t data[], const size_t size) noexcept
  {
    return Seiriakos::deserialize(*this, data, size);
  }
  
  template<typename T, typename... T_>
  void Serializable::serialization(const T& data, const T_&... remaining) const noexcept
  {
    _backend::_serialization_implementation(data);
    serialization(remaining...);
  }

  template<typename T, typename... T_>
  void Serializable::deserialization(T& data, T_&... remaining) noexcept
  {
    _backend::_deserialization_implementation(data);
    deserialization(remaining...);
  }

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

  const char* bytes_as_cstring(const uint8_t data[], const size_t size)
  {
    SEIRIAKOS_THREADLOCAL static std::vector<char> buffer;
    
    if (data == nullptr)
    {
      SEIRIAKOS_LOG("data is nullptr");
      return nullptr;
    }

    if ((3*size + 1) > buffer.capacity())
    {
      buffer = std::vector<char>(3*size + 1);
    }

    buffer.clear();

    for (size_t k = 0; k < size; ++k)
    {
      char nybl_hi = data[k] >> 4;
      char nybl_lo = data[k] & 0xF;

      buffer.push_back((nybl_hi <= 0x9) ? (nybl_hi + '0') : (nybl_hi + 'A' - 10));
      buffer.push_back((nybl_lo <= 0x9) ? (nybl_lo + '0') : (nybl_lo + 'A' - 10));

      buffer.push_back(' ');
    }

    buffer[buffer.size() - 1] = '\0';

    return buffer.data();
  }
//----------------------------------------------------------------------------------------------------------------------
}
#undef SEIRIAKOS_THREADSAFE
#undef SEIRIAKOS_THREADLOCAL
#undef SEIRIAKOS_ATOMIC
#undef SEIRIAKOS_MAKE_MUTEX
#undef SEIRIAKOS_LOCK
#undef SEIRIAKOS_COLD
#undef SEIRIAKOS_ILOG
#undef SEIRIAKOS_LOG
#endif