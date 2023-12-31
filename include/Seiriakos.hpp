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
#include <type_traits> // for std::enable_if, std::is_*
#include <iostream>    // for std::cout, std::cerr
#include <cstring>     // for std::memcpy
#if defined(__STDCPP_THREADS__) and not defined(SEIRIAKOS_NOT_THREADSAFE)
# define  SEIRIAKOS_THREADSAFE
# include <atomic>     // for std::atomic
# include <mutex>      // for std::mutex, std::lock_guard
#endif 
#if defined(SEIRIAKOS_LOGGING)
#if defined (__clang__) or defined(__GNUC__)
# include <cxxabi.h>   // for abi::__cxa_demangle
#endif
# include <typeinfo>   // for typeid
# include <cstdlib>    // for std::free
#endif
//-------------------
#include <array>
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
    MISSING_BYTES       = 1 << 0,
    EMPTY_BUFFER        = 1 << 1,
    SEQUENCE_MISMATCH   = 1 << 2,
    NOT_IMPLEMENTED_YET = 1 << 3
  };

  // serialize "thing"
  template<typename... T> inline
  auto serialize(const T&... things) noexcept -> std::vector<uint8_t>;

  // deserialize data into "thing"
  template<typename... T> inline
  Info deserialize(const uint8_t data[], size_t size, T&... things) noexcept;

  // abstract class to add serialization capabilities
  class Serializable;

  // macro to facilitate serialization/deserialization member function implementations
# define SEIRIAKOS_SEQUENCE(...)

  inline // print bytes from memory
  const char* bytes_as_cstring(const uint8_t data[], const size_t size);

  namespace Global
  {
    static std::ostream out{std::cout.rdbuf()}; // output ostream
    static std::ostream err{std::cerr.rdbuf()}; // error ostream
    static std::ostream log{std::clog.rdbuf()}; // logging ostream
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
# if defined(__clang__)
#   define SEIRIAKOS_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define SEIRIAKOS_CLANG_IGNORE(WARNING, ...)          \
      SEIRIAKOS_PRAGMA(clang diagnostic push)            \
      SEIRIAKOS_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                        \
      SEIRIAKOS_PRAGMA(clang diagnostic pop)
#endif

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define SEIRIAKOS_LIKELY   SEIRIAKOS_CLANG_IGNORE("-Wc++20-extensions", [[likely]])
#   define SEIRIAKOS_UNLIKELY SEIRIAKOS_CLANG_IGNORE("-Wc++20-extensions", [[unlikely]])
# else
#   define SEIRIAKOS_LIKELY   [[likely]]
#   define SEIRIAKOS_UNLIKELY [[unlikely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define SEIRIAKOS_LIKELY   [[likely]]
#   define SEIRIAKOS_UNLIKELY [[unlikely]]
# else
#   define SEIRIAKOS_LIKELY
#   define SEIRIAKOS_UNLIKELY
# endif

// support from clang 3.9.0 and GCC 5.1 onward
# if defined(__clang__)
#   define SEIRIAKOS_MAYBE_UNUSED __attribute__((unused))
# elif defined(__GNUC__)
#   define SEIRIAKOS_MAYBE_UNUSED __attribute__((unused))
# else
#   define SEIRIAKOS_MAYBE_UNUSED
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
    static SEIRIAKOS_THREADLOCAL size_t               _front_of_buffer;
    static SEIRIAKOS_THREADLOCAL Info                 _info;

# if defined(SEIRIAKOS_LOGGING)
    class _indentlog
    {
    public:
      _indentlog(const std::string& text) noexcept
      {
        SEIRIAKOS_MAKE_MUTEX(mtx);
        SEIRIAKOS_LOCK(mtx);

        for (unsigned k = _depth()++; k--;)
        {
          Global::log << "  ";
        }

        Global::log << text << std::endl;
      }

      ~_indentlog() noexcept { --_depth(); }
    private:
      SEIRIAKOS_ATOMIC(unsigned)& _depth()
      {
        static SEIRIAKOS_ATOMIC(unsigned) indentation = {0};
        return indentation;
      };
    };

    template<typename T>
    static
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
#   if defined(__clang__) or defined(__GNUC__)
        return abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
#   else
        return typeid(T).name();
#   endif
      }
    }

#   define SEIRIAKOS_ILOG(text) _backend::_indentlog ilog{text}
#   define SEIRIAKOS_LOG(text)  _backend::_indentlog{text}
# else
#   define SEIRIAKOS_ILOG(text) void(0)
#   define SEIRIAKOS_LOG(text)  void(0)
# endif

    template<typename T>
    using _if_not_Serializable = typename std::enable_if<not std::is_base_of<Serializable, T>::value>::type;
   
    static
    void _serialization_implementation(const Serializable& data);

    static
    void _deserialization_implementation(Serializable& data);

    template<typename T, typename = _if_not_Serializable<T>>
    static
    void _serialization_implementation(const T& data, size_t N = 1)
    {
      SEIRIAKOS_ILOG(_underlying_name<T>() + (N > 1 ? " x" + std::to_string(N) : ""));

      const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(&data);
      _buffer.insert(_buffer.end(), data_ptr, data_ptr + sizeof(T) * N);
    }

    template<typename T, typename = _if_not_Serializable<T>>
    static
    void _deserialization_implementation(T& data, size_t N = 1)
    {
      SEIRIAKOS_ILOG(_underlying_name<T>() + (N > 1 ? " x" + std::to_string(N) : ""));

      if (_front_of_buffer >= _buffer.size()) SEIRIAKOS_UNLIKELY
      {
        _info = Info::EMPTY_BUFFER;
        return;
      }

      if ((_buffer.size() - _front_of_buffer) < (sizeof(T) * N)) SEIRIAKOS_UNLIKELY
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

    SEIRIAKOS_MAYBE_UNUSED
    static
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

    static
    void size_t_deserialization_implementation(size_t& size)
    {
      SEIRIAKOS_ILOG("size_t");

      if (_front_of_buffer >= _buffer.size()) SEIRIAKOS_UNLIKELY
      {
        _info = Info::EMPTY_BUFFER;
        return;
      }

      uint8_t bytes_used = _buffer[_front_of_buffer++];

      if ((_buffer.size() - _front_of_buffer) < bytes_used) SEIRIAKOS_UNLIKELY
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
    static
    void _serialization_implementation(const std::complex<T>& complex);

    template<typename T>
    static
    void _deserialization_implementation(std::complex<T>& complex);

    template<typename T>
    static
    void _serialization_implementation(const std::basic_string<T>& string);

    template<typename T>
    static
    void _deserialization_implementation(std::basic_string<T>& string);

    template<typename T, size_t N>
    static
    void _serialization_implementation(const std::array<T, N>& array);

    template<typename T, size_t N>
    static
    void _deserialization_implementation(std::array<T, N>& array);

    template<typename T>
    static
    void _serialization_implementation(const std::vector<T>& vector);

    template<typename T>
    static
    void _deserialization_implementation(std::vector<T>& vector);

    template<typename T>
    static
    void _serialization_implementation(const std::list<T>& list);

    template<typename T>
    static
    void _deserialization_implementation(std::list<T>& list);

    template<typename T>
    static
    void _serialization_implementation(const std::deque<T>& deque);

    template<typename T>
    static
    void _deserialization_implementation(std::deque<T>& deque);

    template<typename T1, typename T2>
    static
    void _serialization_implementation(const std::pair<T1, T2>& pair);

    template<typename T1, typename T2>
    static
    void _deserialization_implementation(std::pair<T1, T2>& pair);

    template<typename T1, typename T2>
    static
    void _serialization_implementation(const std::unordered_map<T1, T2>& unordered_map);

    template<typename T1, typename T2>
    static
    void _deserialization_implementation(std::unordered_map<T1, T2>& unordered_map);

    template<typename T1, typename T2>
    static
    void _serialization_implementation(const std::unordered_multimap<T1, T2>& unordered_multimap);

    template<typename T1, typename T2>
    static
    void _deserialization_implementation(std::unordered_multimap<T1, T2>& unordered_multimap);

    template<typename T1, typename T2>
    static
    void _serialization_implementation(const std::map<T1, T2>& map);

    template<typename T1, typename T2>
    static
    void _deserialization_implementation(std::map<T1, T2>& map);

    template<typename T1, typename T2>
    static
    void _serialization_implementation(const std::multimap<T1, T2>& multimap);

    template<typename T1, typename T2>
    static
    void _deserialization_implementation(std::multimap<T1, T2>& multimap);

    template<typename T>
    static
    void _serialization_implementation(const std::unordered_set<T>& unordered_set);

    template<typename T>
    static
    void _deserialization_implementation(std::unordered_set<T>& unordered_set);

    template<typename T>
    static
    void _serialization_implementation(const std::unordered_multiset<T>& unordered_multiset);

    template<typename T>
    static
    void _deserialization_implementation(std::unordered_multiset<T>& unordered_multiset);

    template<typename T>
    static
    void _serialization_implementation(const std::set<T>& set);

    template<typename T>
    static
    void _deserialization_implementation(std::set<T>& set);

    template<typename T>
    static
    void _serialization_implementation(const std::multiset<T>& multiset);

    template<typename T>
    static
    void _deserialization_implementation(std::multiset<T>& multiset);

    template<typename... T>
    static
    void _serialization_implementation(const std::tuple<T...>& tuple);

    template<typename... T>
    static
    void _deserialization_implementation(std::tuple<T...>& tuple);

    template<typename T, typename... T_>
    inline
    auto _sizeof_things() -> typename std::enable_if<sizeof...(T_) == 0, size_t>::type
    {
      return sizeof(T);
    }

    template<typename T, typename... T_>
    inline
    auto _sizeof_things() -> typename std::enable_if<sizeof...(T_) != 0, size_t>::type
    {
      return sizeof(T) + _sizeof_things<T_...>();
    }

    inline
    void _serialize_things()
    {}

    template<typename T, typename... T_>
    inline
    void _serialize_things(const T& thing, const T_&... remaining_things)
    {
      _serialization_implementation(thing);
      _serialize_things(remaining_things...);
    }

    inline
    void _deserialize_things()
    {}

    template<typename T, typename... T_>
    inline
    void _deserialize_things(T& thing, T_&... remaining_things)
    {
      _deserialization_implementation(thing);
      _deserialize_things(remaining_things...);
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  class Serializable
  {
  public:
    inline // serialize *this according to serialization_sequence
    auto serialize() const noexcept -> std::vector<uint8_t>;

    inline // deserialize into *this according to deserialization_sequence
    Info deserialize(const uint8_t data[], const size_t size) noexcept;
  protected:
    virtual // serialization sequence (provided by the inheriting class)
    void serialization_sequence() const noexcept = 0;
    
    virtual // deserialization sequence (provided by the inheriting class)
    void deserialization_sequence()     noexcept = 0;

    template<typename T, typename... T_>
    inline // recursive calls to appropriate _serialization_implementation overloads
    void serialization(const T& data, const T_&... remaining_data) const noexcept;
    void serialization()                                           const noexcept
    {}

    template<typename T, typename... T_>
    inline // recursive calls to appropriate _deserialization_implementation overloads
    void deserialization(T& data, T_&... remaining_data) noexcept;
    void deserialization()                               noexcept
    {}
    
  friend void _backend::_serialization_implementation(const Serializable& data);
  friend void _backend::_deserialization_implementation(Serializable& data);
  };
//----------------------------------------------------------------------------------------------------------------------
  namespace _backend
  {
    SEIRIAKOS_MAYBE_UNUSED
    static
    void _serialization_implementation(const Serializable& data)
    {
      SEIRIAKOS_ILOG("Serializable");
      
      data.serialization_sequence(); // serialize data via its specialized implementation
    }

    static
    void _deserialization_implementation(Serializable& data)
    {
      SEIRIAKOS_ILOG("Serializable");

      data.deserialization_sequence(); // serialize data via its specialized implementation
    }

    template<typename T>
    static
    void _serialization_implementation(const std::complex<T>& complex)
    {
      SEIRIAKOS_ILOG("std::complex");

      _serialization_implementation(complex.real);
      _serialization_implementation(complex.imag);
    }

    template<typename T>
    static
    void _deserialization_implementation(std::complex<T>& complex)
    {
      SEIRIAKOS_ILOG("std::complex");
      
      _deserialization_implementation(complex.real);
      _deserialization_implementation(complex.imag);
    }

    template<typename T>
    static
    void _serialization_implementation(const std::basic_string<T>& string)
    {
      SEIRIAKOS_ILOG("std::basic_string");

      size_t_serialization_implementation(string.size());

      if (std::is_fundamental<T>::value) SEIRIAKOS_LIKELY
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
    static
    void _deserialization_implementation(std::basic_string<T>& string)
    {
      SEIRIAKOS_ILOG("std::basic_string");

      size_t size = {};
      size_t_deserialization_implementation(size);

      string.resize(size);

      if (std::is_fundamental<T>::value) SEIRIAKOS_LIKELY
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
    static
    void _serialization_implementation(const std::array<T, N>& array)
    {
      SEIRIAKOS_ILOG("std::array");

      if (std::is_fundamental<T>::value) SEIRIAKOS_LIKELY
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
    static
    void _deserialization_implementation(std::array<T, N>& array)
    {
      SEIRIAKOS_ILOG("std::array");

      if (std::is_fundamental<T>::value) SEIRIAKOS_LIKELY
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
    static
    void _serialization_implementation(const std::vector<T>& vector)
    {
      SEIRIAKOS_ILOG("std::vector");

      size_t_serialization_implementation(vector.size());

      if (std::is_fundamental<T>::value) SEIRIAKOS_LIKELY
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
    static
    void _deserialization_implementation(std::vector<T>& vector)
    {
      SEIRIAKOS_ILOG("std::vector");

      size_t size = {};
      size_t_deserialization_implementation(size);

      vector.resize(size);

      if (std::is_fundamental<T>::value) SEIRIAKOS_LIKELY
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
    static
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
    static
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
    static
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
    static
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
    static
    void _serialization_implementation(const std::pair<T1, T2>& pair)
    {
      SEIRIAKOS_ILOG("std::pair");

      _serialization_implementation(pair.first);
      _serialization_implementation(pair.second);
    }

    template<typename T1, typename T2>
    static
    void _deserialization_implementation(std::pair<T1, T2>& pair)
    {
      SEIRIAKOS_ILOG("std::pair");

      _deserialization_implementation(pair.first);
      _deserialization_implementation(pair.second);
    }

    template<typename T1, typename T2>
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
    static
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
      static inline
      void implementation(const std::tuple<T...>& tuple);
    };

    template<typename... T>
    struct _tuple_serialization<0, T...>
    {
      static
      void implementation(const std::tuple<T...>&)
      {}
    };

    template<size_t N, typename... T>
    void _tuple_serialization<N, T...>::implementation(const std::tuple<T...>& tuple)
    {
      _serialization_implementation(std::get<sizeof...(T) - N>(tuple));
      _tuple_serialization<N-1, T...>::implementation(tuple);
    }

    template<typename... T>
    static
    void _serialization_implementation(const std::tuple<T...>& tuple)
    {
      SEIRIAKOS_ILOG("std::tuple");

      _tuple_serialization<sizeof...(T), T...>::implementation(tuple);
    }

    template<size_t N, typename... T>
    struct _tuple_deserialization
    {
      static inline
      void implementation(std::tuple<T...>& tuple);
    };

    template<typename... T>
    struct _tuple_deserialization<0, T...>
    {
      static void implementation(std::tuple<T...>&)
      {}
    };

    template<size_t N, typename... T>
    void _tuple_deserialization<N, T...>::implementation(std::tuple<T...>& tuple)
    {
      _deserialization_implementation(std::get<sizeof...(T) - N>(tuple));
      _tuple_deserialization<N-1, T...>::implementation(tuple);
    }

    template<typename... T>
    static
    void _deserialization_implementation(std::tuple<T...>& tuple)
    {
      SEIRIAKOS_ILOG("std::tuple");

      _tuple_deserialization<sizeof...(T), T...>::implementation(tuple);
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  template<typename... T>
  auto serialize(const T&... things) noexcept -> std::vector<uint8_t>
  {
    SEIRIAKOS_LOG("----serialization summary:");
    _backend::_buffer.clear();
    _backend::_buffer.reserve(_backend::_sizeof_things<T...>());
    _backend::_serialize_things(things...);
    SEIRIAKOS_LOG("----------------------------");
    return _backend::_buffer;
  }

  template<typename... T>
  Info deserialize(const uint8_t data[], size_t size, T&... things) noexcept
  {
    SEIRIAKOS_LOG("----deserialization summary:");
    _backend::_buffer.assign(data, data + size);
    _backend::_front_of_buffer = 0;
    _backend::_info = Info::ALL_GOOD;
    _backend::_deserialize_things(things...);

    if (_backend::_info == Info::ALL_GOOD) SEIRIAKOS_LIKELY
    {
      if (_backend::_front_of_buffer != _backend::_buffer.size()) SEIRIAKOS_UNLIKELY
      {
        _backend::_info = Info::SEQUENCE_MISMATCH;
      }
    }

    SEIRIAKOS_LOG("----------------------------");

    return _backend::_info;
  }
//----------------------------------------------------------------------------------------------------------------------
  auto Serializable::serialize() const noexcept -> std::vector<uint8_t>
  {
    return Seiriakos::serialize(*this);
  }

  Info Serializable::deserialize(const uint8_t data[], const size_t size) noexcept
  {
    return Seiriakos::deserialize(data, size, *this);
  }
  
  template<typename T, typename... T_>
  void Serializable::serialization(const T& data, const T_&... remaining_data) const noexcept
  {
    _backend::_serialization_implementation(data);
    serialization(remaining_data...);
  }

  template<typename T, typename... T_>
  void Serializable::deserialization(T& data, T_&... remaining_data) noexcept
  {
    _backend::_deserialization_implementation(data);
    deserialization(remaining_data...);
  }
//----------------------------------------------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------------------------------------
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
#undef SEIRIAKOS_PRAGMA
#undef SEIRIAKOS_CLANG_IGNORE
#undef SEIRIAKOS_THREADSAFE
#undef SEIRIAKOS_THREADLOCAL
#undef SEIRIAKOS_ATOMIC
#undef SEIRIAKOS_MAKE_MUTEX
#undef SEIRIAKOS_LOCK
#undef SEIRIAKOS_LIKELY
#undef SEIRIAKOS_UNLIKELY
#undef SEIRIAKOS_ILOG
#undef SEIRIAKOS_LOG
#endif