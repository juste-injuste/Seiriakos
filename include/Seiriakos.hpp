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
TODO:
  std::vector<bool>
  std::type_index
  std::valarray
  std::bitset
  std::queue
  std::priority_queue
  std::forward_list
  std::stack
  std::ratio
  std::chrono::duration
  std::regex
  std::atomic

  std::system_error
  std::hash ?? idk, im not sure about how it works
  std::basic_stringbuf ??
  std::basic_istringstream ??
  std::basic_ostringstream ??
  std::basic_stringstream ??
  std::locale ??
  std::ios ??
-----description------------------------------------------------------------------------------------

Seiriakos is a simple and lightweight C++11 (and newer) library that allows you serialize and
deserialize objects.

-----inclusion guard------------------------------------------------------------------------------*/
#ifndef _seiriakos_hpp
#define _seiriakos_hpp
//---necessary standard libraries-------------------------------------------------------------------
#include <cstddef>     // for size_t
#include <cstdint>     // for uint8_t
#include <vector>      // for std::vector
#include <type_traits> // for std::enable_if, std::is_*
#include <iostream>    // for std::clog
#include <cstring>     // for std::memcpy
//---conditionally necessary standard libraries-----------------------------------------------------
#if defined(__STDCPP_THREADS__) and not defined(SRZ_NOT_THREADSAFE)
# define  _srz_impl_THREADSAFE
# include <atomic> // for std::atomic
# include <mutex>  // for std::mutex, std::lock_guard
#endif
#if defined(SRZ_DEBUGGING)
#if defined (__clang__) or defined(__GNUC__)
# include <cxxabi.h> // for abi::__cxa_demangle
#endif
# include <typeinfo> // for typeid
# include <cstdlib>  // for std::free
#endif
//-------------------

#include <array>          // for std::array
#include <complex>        // for std::complex
#include <list>           // for std::list
#include <deque>          // for std::deque
#include <string>         // for std::basic_string
#include <utility>        // for std::pair
#include <unordered_map>  // for std::unordered_map, std::unordered_multimap
#include <map>            // for std::map, std::multimap
#include <unordered_set>  // for std::unordered_set, std::unordered_multiset
#include <set>            // for std::set, std::multiset
#include <tuple>          // for std::tuple
//

// things that make no sens i think
// #include <streambuf>         // what's a basic_streambuf
// #include <ostream>           // what's a basic_ostream
// #include <functional>        // except maybe for the things like std::plus ??
// #include <initializer_list>
// #include <iterator>
// #include <fstream>
// #include <exception>
// #include <istream>
// #include <new>
// #include <stdexcept>
// #include <memory> // std::unique_ptr, std::shared_ptr, std::weak_ptr
// #include <new>
// #include <thread // thread/jthread
// #include <future>
// #include <scoped_allocator>
// #include <mutex>
// #include <condition_variable>
//---Seiriakos library------------------------------------------------------------------------------
namespace srz
{
  class Info;

  // enum class Info : uint8_t
  // {
  //   ALL_GOOD            = 0,
  //   MISSING_BYTES       = 1 << 0,
  //   EMPTY_BUFFER        = 1 << 1,
  //   SEQUENCE_MISMATCH   = 1 << 2,
  //   NOT_IMPLEMENTED_YET = 1 << 3
  // };

  template<typename... T>
  inline // serialize 'things'
  auto serialize(const T&... things) noexcept -> std::vector<uint8_t>;

  template<typename... T>
  inline // deserialize into 'things'
  Info deserialize(const uint8_t data[], size_t size, T&... things) noexcept;

  // abstract class to add serialization capabilities
  class Serializable;

  // macro to facilitate serialization/deserialization member function implementations
# define SRZ_SERIALIZATION_SEQUENCE(...)

  inline // print bytes from memory
  auto bytes_as_cstring(const uint8_t data[], const size_t size) -> const char*;

  namespace _io
  {
    static std::ostream dbg(std::clog.rdbuf()); // debugging
  }

  namespace _version
  {
    constexpr unsigned long MAJOR  = 000;
    constexpr unsigned long MINOR  = 001;
    constexpr unsigned long PATCH  = 000;
    constexpr unsigned long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
//----------------------------------------------------------------------------------------------------------------------
  class Info
  {
  public:
    enum Code : uint8_t
    {
      ALL_GOOD           ,
      MISSING_BYTES      ,
      EMPTY_BUFFER       ,
      SEQUENCE_MISMATCH  ,
      NOT_IMPLEMENTED_YET
    };

    Info() = delete;
    constexpr Info(Code code) noexcept : _code(code) { }

    void operator=(Code code) noexcept
    {
      if (_code != Info::ALL_GOOD) _code = code;
    }

    constexpr operator Code() const noexcept { return _code; }
    constexpr const char* description() const { return "text"; }
    operator bool() const = delete;

  private:
    Code _code;
  };
//----------------------------------------------------------------------------------------------------------------------
  namespace _impl
  {
# if defined(__clang__)
#   define _srz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define _srz_impl_CLANG_IGNORE(WARNING, ...)          \
      _srz_impl_PRAGMA(clang diagnostic push)            \
      _srz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                        \
      _srz_impl_PRAGMA(clang diagnostic pop)
#endif

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define _srz_impl_LIKELY   _srz_impl_CLANG_IGNORE("-Wc++20-extensions", [[likely]])
#   define _srz_impl_UNLIKELY _srz_impl_CLANG_IGNORE("-Wc++20-extensions", [[unlikely]])
# else
#   define _srz_impl_LIKELY   [[likely]]
#   define _srz_impl_UNLIKELY [[unlikely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _srz_impl_LIKELY   [[likely]]
#   define _srz_impl_UNLIKELY [[unlikely]]
# else
#   define _srz_impl_LIKELY
#   define _srz_impl_UNLIKELY
# endif

// support from clang 3.9.0 and GCC 4.7.3 onward
# if defined(__clang__)
#   define _srz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _srz_impl_LIKELY
#   define _srz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _srz_impl_UNLIKELY
# elif defined(__GNUC__)
#   define _srz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _srz_impl_LIKELY
#   define _srz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _srz_impl_UNLIKELY
# else
#   define _srz_impl_EXPECTED(CONDITION) (CONDITION) _srz_impl_LIKELY
#   define _srz_impl_ABNORMAL(CONDITION) (CONDITION) _srz_impl_UNLIKELY
# endif

// support from clang 3.9.0 and GCC 5.1 onward
# if defined(__clang__)
#   define _srz_impl_NODISCARD    __attribute__((warn_unused_result))
#   define _srz_impl_MAYBE_UNUSED __attribute__((unused))
# elif defined(__GNUC__)
#   define _srz_impl_NODISCARD    __attribute__((warn_unused_result))
#   define _srz_impl_MAYBE_UNUSED __attribute__((unused))
# else
#   define _srz_impl_NODISCARD
#   define _srz_impl_MAYBE_UNUSED
# endif

// support from clang 10.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 10)
# if __cplusplus < 202002L
#   define _srz_impl_NODISCARD_REASON(REASON) _srz_impl_CLANG_IGNORE("-Wc++20-extensions", [[nodiscard(REASON)]])
# else
#   define _srz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _srz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# else
#   define _srz_impl_NODISCARD_REASON(REASON) _srz_impl_NODISCARD
# endif

# if __cplusplus >= 201402L
#   define _srz_impl_CONSTEXPR_CPP14 constexpr
# else
#   define _srz_impl_CONSTEXPR_CPP14
# endif

# if __cplusplus >= 201703L
#   define _srz_impl_CONSTEXPR_CPP17 constexpr
# else
#   define _srz_impl_CONSTEXPR_CPP17
# endif

# if defined(_srz_impl_THREADSAFE)
#   undef  _srz_impl_THREADSAFE
#   define _srz_impl_THREADLOCAL         thread_local
#   define _srz_impl_ATOMIC(T)           std::atomic<T>
#   define _srz_impl_DECLARE_MUTEX(...)  static std::mutex __VA_ARGS__
#   define _srz_impl_DECLARE_LOCK(MUTEX) std::lock_guard<decltype(MUTEX)> _lock{MUTEX}
# else
#   define _srz_impl_THREADLOCAL
#   define _srz_impl_ATOMIC(T)           T
#   define _srz_impl_DECLARE_MUTEX(...)
#   define _srz_impl_DECLARE_LOCK(MUTEX)
# endif

    static _srz_impl_THREADLOCAL std::vector<uint8_t> _buffer;
    static _srz_impl_THREADLOCAL size_t               _front_of_buffer;
    static _srz_impl_THREADLOCAL Info                 _info = Info::ALL_GOOD;

# if defined(SRZ_DEBUGGING)
    template<typename T>
    auto _underlying_name() -> const char*
    {
      static _srz_impl_THREADLOCAL char _underlying_name_buffer[256];

      if (std::is_integral<T>::value)
      {
        if (std::is_unsigned<T>::value)
        {
          std::sprintf(_underlying_name_buffer, "uint%u", unsigned(sizeof(T) * 8));
        }
        else
        {
          std::sprintf(_underlying_name_buffer, "int%u", unsigned(sizeof(T) * 8));
        }
      }
      else if (std::is_floating_point<T>::value)
      {
        std::sprintf(_underlying_name_buffer, "float%u", unsigned(sizeof(T) * 8));
      }
      else
      {
#   if defined(__clang__) or defined(__GNUC__)
        static _srz_impl_THREADLOCAL size_t size = sizeof(_underlying_name_buffer);
        abi::__cxa_demangle(typeid(T).name(), _underlying_name_buffer, &size, nullptr);
#   else
        std::sprintf(_underlying_name_buffer, "%s", typeid(T).name());
#   endif
      }

      return _underlying_name_buffer;
    }

    _srz_impl_DECLARE_MUTEX(_dbg_mtx);
    _srz_impl_MAYBE_UNUSED static _srz_impl_THREADLOCAL char _dbg_buffer[256] = {};

    class _indentdebug
    {
    public:
      template<typename... T>
      _srz_impl_CONSTEXPR_CPP14
      _indentdebug(T... arguments) noexcept
      {
        _srz_impl_DECLARE_LOCK(_impl::_dbg_mtx);

        for (unsigned k = _depth()++; k--;)
        {
          _io::dbg << "  ";
        }

        std::sprintf(_dbg_buffer, arguments...);

        _io::dbg << _dbg_buffer << std::endl;
      }

      ~_indentdebug() noexcept { --_depth(); }
    private:
      _srz_impl_ATOMIC(unsigned)& _depth()
      {
        static _srz_impl_ATOMIC(unsigned) indentation = {0};
        return indentation;
      };
    };

#   define _srz_impl_IDEBUGGING(...) _impl::_indentdebug _idbg(__VA_ARGS__)
#   define _srz_impl_DEBUGGING(...)                                    \
      [&](const char* const caller){                                   \
        _srz_impl_DECLARE_LOCK(_impl::_dbg_mtx);                       \
        std::sprintf(_impl::_dbg_buffer, __VA_ARGS__);                 \
        _io::dbg << caller << ": " << _impl::_dbg_buffer << std::endl; \
      }(__func__)
# else
#   define _srz_impl_IDEBUGGING(...) void(0)
#   define _srz_impl_DEBUGGING(...)  void(0)
# endif

# if defined(SRZ_UNSAFE)
#   define _srz_impl_SAFE(...)   
#   define _srz_impl_UNSAFE(...) __VA_ARGS__
# else
#   define _srz_impl_SAFE(...)   __VA_ARGS__
#   define _srz_impl_UNSAFE(...)
#endif

    template<typename T>
    using _if_not_Serializable = typename std::enable_if<not std::is_base_of<Serializable, T>::value>::type;

    template<typename T, typename = _if_not_Serializable<T>>
    constexpr
    void _serialization_implementation(const T& data_, const size_t N_ = 1)
    {
      _srz_impl_IDEBUGGING("%s x%u", _underlying_name<T>(),  N_);

      const auto data_ptr = reinterpret_cast<const uint8_t*>(&data_);
      _buffer.insert(_buffer.end(), data_ptr, data_ptr + sizeof(T) * N_);
    }

    template<typename T, typename = _if_not_Serializable<T>>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(T& data_, const size_t N_ = 1)
    {
      _srz_impl_IDEBUGGING("%s x%u", _underlying_name<T>(),  N_);

      _srz_impl_SAFE(
        if _srz_impl_ABNORMAL(_front_of_buffer >= _buffer.size())
        {
          _info = Info::EMPTY_BUFFER;
          return;
        }

        if _srz_impl_ABNORMAL((_buffer.size() - _front_of_buffer) < (sizeof(T) * N_))
        {
          _info =  Info::MISSING_BYTES;
          return;
        }
      )

      // set data's bytes one by one from the front of the buffer
      const auto data_ptr   = reinterpret_cast<uint8_t*>(&data_);
      const auto buffer_ptr = _buffer.data() + _front_of_buffer;
      std::memcpy(data_ptr, buffer_ptr, sizeof(T) * N_);

      _front_of_buffer += sizeof(T) * N_;
    }

    _srz_impl_MAYBE_UNUSED
    static
    void size_t_serialization_implementation(size_t size_)
    {
#   if defined(SRZ_FIXED_LENGHT)
      _serialization_implementation(size_);
#   else
      _srz_impl_IDEBUGGING("size_t");

      uint8_t bytes_used = 1;
      for (size_t k = size_; k >>= 8; ++bytes_used) {}

      _buffer.push_back(bytes_used);

      for (size_t k = 0; bytes_used--; k += 8)
      {
        _buffer.push_back(static_cast<uint8_t>((size_ >> k) & 0xFF));
      }
#   endif
    }

    _srz_impl_MAYBE_UNUSED
    static
    void size_t_deserialization_implementation(size_t& size_)
    {
#   if defined(SRZ_FIXED_LENGHT)
      _deserialization_implementation(size_);
#   else
      _srz_impl_IDEBUGGING("size_t");

      _srz_impl_SAFE(
        if _srz_impl_ABNORMAL(_front_of_buffer >= _buffer.size())
        {
          _info = Info::EMPTY_BUFFER;
          return;
        }
      )

      uint8_t bytes_used = _buffer[_front_of_buffer++];

      _srz_impl_SAFE(
        if _srz_impl_ABNORMAL((_buffer.size() - _front_of_buffer) < bytes_used)
        {
          _info = Info::MISSING_BYTES;
          return;
        }
      )

      size_ = 0;
      for (size_t k = 0; bytes_used--; k += 8)
      {
        size_ |= (_buffer[_front_of_buffer++] << k);
      }
#   endif
    }

    inline
    void _serialization_implementation(const Serializable& serializable);

    inline
    void _deserialization_implementation(Serializable& serializable);

    template<typename T>
    constexpr
    void _serialization_implementation(const std::complex<T>& complex);

    template<typename T>
    constexpr
    void _deserialization_implementation(std::complex<T>& complex);

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::basic_string<T>& string);

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::basic_string<T>& string);

    template<typename T, size_t N>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::array<T, N>& array);

    template<typename T, size_t N>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::array<T, N>& array);

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::vector<T>& vector);

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::vector<T>& vector);

    template<typename T>
    constexpr
    void _serialization_implementation(const std::list<T>& list);

    template<typename T>
    constexpr
    void _deserialization_implementation(std::list<T>& list);

    template<typename T>
    constexpr
    void _serialization_implementation(const std::deque<T>& deque);

    template<typename T>
    constexpr
    void _deserialization_implementation(std::deque<T>& deque);

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::pair<T1, T2>& pair);

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::pair<T1, T2>& pair);

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::unordered_map<T1, T2>& unordered_map);

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::unordered_map<T1, T2>& unordered_map);

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::unordered_multimap<T1, T2>& unordered_multimap);

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::unordered_multimap<T1, T2>& unordered_multimap);

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::map<T1, T2>& map);

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::map<T1, T2>& map);

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::multimap<T1, T2>& multimap);

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::multimap<T1, T2>& multimap);

    template<typename T>
    constexpr
    void _serialization_implementation(const std::unordered_set<T>& unordered_set);

    template<typename T>
    constexpr
    void _deserialization_implementation(std::unordered_set<T>& unordered_set);

    template<typename T>
    constexpr
    void _serialization_implementation(const std::unordered_multiset<T>& unordered_multiset);

    template<typename T>
    constexpr
    void _deserialization_implementation(std::unordered_multiset<T>& unordered_multiset);

    template<typename T>
    constexpr
    void _serialization_implementation(const std::set<T>& set);

    template<typename T>
    constexpr
    void _deserialization_implementation(std::set<T>& set);

    template<typename T>
    constexpr
    void _serialization_implementation(const std::multiset<T>& multiset);

    template<typename T>
    constexpr
    void _deserialization_implementation(std::multiset<T>& multiset);

    template<typename... T>
    constexpr
    void _serialization_implementation(const std::tuple<T...>& tuple);

    template<typename... T>
    constexpr
    void _deserialization_implementation(std::tuple<T...>& tuple);

    template<typename T, typename... T_>
    constexpr
    auto _sizeof_things() -> typename std::enable_if<sizeof...(T_) == 0, size_t>::type
    {
      return sizeof(T);
    }

    template<typename T, typename... T_>
    constexpr
    auto _sizeof_things() -> typename std::enable_if<sizeof...(T_) != 0, size_t>::type
    {
      return sizeof(T) + _sizeof_things<T_...>();
    }

    inline _srz_impl_CONSTEXPR_CPP14 void _serialize_things() {}

    template<typename T, typename... T_>
    constexpr
    void _serialize_things(const T& thing, const T_&... remaining_things)
    {
      _serialization_implementation(thing);
      _serialize_things(remaining_things...);
    }

    inline _srz_impl_CONSTEXPR_CPP14 void _deserialize_things() {}

    template<typename T, typename... T_>
    constexpr
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
    _srz_impl_NODISCARD_REASON("serialize: ignoring the return value makes no sens")
    inline // serialize *this according to serialization_sequence
    auto serialize() const noexcept -> std::vector<uint8_t>;

    inline // deserialize into *this according to deserialization_sequence
    Info deserialize(const uint8_t data[], size_t size) noexcept;
  protected:
    virtual // serialization sequence (provided by the inheriting class)
    void serialization_sequence() const noexcept = 0;

    virtual // deserialization sequence (provided by the inheriting class)
    void deserialization_sequence() noexcept = 0;

    template<typename... T>
    inline // recursive calls to appropriate serialization overloads
    void serialization(const T&... data) const noexcept;

    template<typename... T>
    inline // recursive calls to appropriate deserialization overloads
    void deserialization(T&... data) noexcept;

    friend void _impl::_serialization_implementation(const Serializable& data);
    friend void _impl::_deserialization_implementation(Serializable& data);
  };
//----------------------------------------------------------------------------------------------------------------------
  namespace _impl
  {
    void _serialization_implementation(const Serializable& serializable_)
    {
      _srz_impl_IDEBUGGING("Serializable");

      serializable_.serialization_sequence();
    }

    void _deserialization_implementation(Serializable& serializable_)
    {
      _srz_impl_IDEBUGGING("Serializable");

      serializable_.deserialization_sequence();
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::complex<T>& complex_)
    {
      _srz_impl_IDEBUGGING("std::complex<%s>", _underlying_name<T>());

      _serialization_implementation(complex_.real);
      _serialization_implementation(complex_.imag);
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::complex<T>& complex)
    {
      _srz_impl_IDEBUGGING("std::complex<%s>", _underlying_name<T>());

      _deserialization_implementation(complex.real);
      _deserialization_implementation(complex.imag);
    }

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::basic_string<T>& string_)
    {
      _srz_impl_IDEBUGGING("std::basic_string<%s>", _underlying_name<T>());

      size_t_serialization_implementation(string_.size());

      if _srz_impl_CONSTEXPR_CPP17 _srz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _serialization_implementation(string_[0], string_.size());
      }
      else
      {
        for (const auto character : string_)
        {
          _serialization_implementation(character);
        }
      }
    }

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::basic_string<T>& string_)
    {
      _srz_impl_IDEBUGGING("std::basic_string<%s>", _underlying_name<T>());

      size_t size = {};
      size_t_deserialization_implementation(size);

      string_.resize(size);

      if _srz_impl_CONSTEXPR_CPP17 _srz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _deserialization_implementation(string_[0], size);
      }
      else
      {
        for (auto& character : string_)
        {
          _deserialization_implementation(character);
        }
      }
    }

    template<typename T, size_t N>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::array<T, N>& array_)
    {
      _srz_impl_IDEBUGGING("std::array<%s, %u>", _underlying_name<T>(), unsigned(N));

      if _srz_impl_CONSTEXPR_CPP17 _srz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _serialization_implementation(array_[0], N);
      }
      else
      {
        for (const auto& value : array_)
        {
          _serialization_implementation(value);
        }
      }
    }

    template<typename T, size_t N>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::array<T, N>& array_)
    {
      _srz_impl_IDEBUGGING("std::array<%s, %u>", _underlying_name<T>(), unsigned(N));

      if _srz_impl_CONSTEXPR_CPP17 _srz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _deserialization_implementation(array_[0], N);
      }
      else
      {
        for (auto& value : array_)
        {
          _deserialization_implementation(value);
        }
      }
    }

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::vector<T>& vector_)
    {
      _srz_impl_IDEBUGGING("std::vector<%s>", _underlying_name<T>());

      size_t_serialization_implementation(vector_.size());

      if _srz_impl_CONSTEXPR_CPP17 _srz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _serialization_implementation(vector_[0], vector_.size());
      }
      else
      {
        for (const auto& value : vector_)
        {
          _serialization_implementation(value);
        }
      }
    }

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::vector<T>& vector_)
    {
      _srz_impl_IDEBUGGING("std::vector<%s>", _underlying_name<T>());

      size_t size = {};
      size_t_deserialization_implementation(size);

      vector_.resize(size);

      if _srz_impl_CONSTEXPR_CPP17 _srz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _deserialization_implementation(vector_[0], size);
      }
      else
      {
        for (auto& value : vector_)
        {
          _deserialization_implementation(value);
        }
      }
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::list<T>& list_)
    {
      _srz_impl_IDEBUGGING("std::list<%s>", _underlying_name<T>());

      size_t_serialization_implementation(list_.size());

      for (const auto& value : list_)
      {
        _serialization_implementation(value);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::list<T>& list_)
    {
      _srz_impl_IDEBUGGING("std::list<%s>", _underlying_name<T>());

      size_t size = {};
      size_t_deserialization_implementation(size);

      list_.resize(size);
      for (auto& value : list_)
      {
        _deserialization_implementation(value);
      }
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::deque<T>& deque_)
    {
      _srz_impl_IDEBUGGING("std::deque<%s>", _underlying_name<T>());

      size_t_serialization_implementation(deque_.size());

      for (const auto& value : deque_)
      {
        _serialization_implementation(value);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::deque<T>& deque_)
    {
      _srz_impl_IDEBUGGING("std::deque<%s>", _underlying_name<T>());

      size_t size = {};
      size_t_deserialization_implementation(size);

      deque_.resize(size);
      for (auto& value : deque_)
      {
        _deserialization_implementation(value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::pair<T1, T2>& pair_)
    {
      _srz_impl_IDEBUGGING("std::pair<%s, %s>", _underlying_name<T1>(), _underlying_name<T2>());

      _serialization_implementation(pair_.first);
      _serialization_implementation(pair_.second);
    }

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::pair<T1, T2>& pair_)
    {
      _srz_impl_IDEBUGGING("std::pair<%s, %s>", _underlying_name<T1>(), _underlying_name<T2>());

      _deserialization_implementation(pair_.first);
      _deserialization_implementation(pair_.second);
    }

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::unordered_map<T1, T2>& unordered_map_)
    {
      _srz_impl_IDEBUGGING("std::unordered_map");

      size_t_serialization_implementation(unordered_map_.size());

      for (const auto& key_value : unordered_map_)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::unordered_map<T1, T2>& unordered_map_)
    {
      _srz_impl_IDEBUGGING("std::unordered_map");

      size_t size = {};
      size_t_deserialization_implementation(size);

      unordered_map_.clear();
      unordered_map_.reserve(size);

      std::pair<T1, T2> key_value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key_value);
        unordered_map_.insert(std::move(key_value));
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::unordered_multimap<T1, T2>& unordered_multimap_)
    {
      _srz_impl_IDEBUGGING("std::unordered_multimap");

      size_t_serialization_implementation(unordered_multimap_.size());

      for (const auto& key_value : unordered_multimap_)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::unordered_multimap<T1, T2>& unordered_multimap_)
    {
      _srz_impl_IDEBUGGING("std::unordered_multimap");

      size_t size = {};
      size_t_deserialization_implementation(size);

      unordered_multimap_.clear();
      unordered_multimap_.reserve(size);

      std::pair<T1, T2> key_value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key_value);
        unordered_multimap_.insert(std::move(key_value));
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::map<T1, T2>& map_)
    {
      _srz_impl_IDEBUGGING("std::map");

      size_t_serialization_implementation(map_.size());

      for (const auto& key_value : map_)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::map<T1, T2>& map_)
    {
      _srz_impl_IDEBUGGING("std::map");

      size_t size = {};
      size_t_deserialization_implementation(size);

      map_.clear();

      std::pair<T1, T2> key_value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key_value);
        map_.insert(std::move(key_value));
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::multimap<T1, T2>& multimap_)
    {
      _srz_impl_IDEBUGGING("std::multimap");

      size_t_serialization_implementation(multimap_.size());

      for (const auto& key_value : multimap_)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::multimap<T1, T2>& multimap_)
    {
      _srz_impl_IDEBUGGING("std::multimap");

      size_t size = {};
      size_t_deserialization_implementation(size);

      multimap_.clear();

      std::pair<T1, T2> key_value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key_value);
        multimap_.insert(std::move(key_value));
      }
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::unordered_set<T>& unordered_set_)
    {
      _srz_impl_IDEBUGGING("std::unordered_set");

      size_t_serialization_implementation(unordered_set_.size());

      for (const auto& key : unordered_set_)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::unordered_set<T>& unordered_set_)
    {
      _srz_impl_IDEBUGGING("std::unordered_set");

      size_t size = {};
      size_t_deserialization_implementation(size);

      unordered_set_.clear();
      unordered_set_.reserve(size);

      T key = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key);
        unordered_set_.insert(std::move(key));
      }
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::unordered_multiset<T>& unordered_multiset_)
    {
      _srz_impl_IDEBUGGING("std::unordered_multiset");

      size_t_serialization_implementation(unordered_multiset_.size());

      for (const auto& key : unordered_multiset_)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::unordered_multiset<T>& unordered_multiset_)
    {
      _srz_impl_IDEBUGGING("std::unordered_multiset");

      size_t size = {};
      size_t_deserialization_implementation(size);

      unordered_multiset_.clear();
      unordered_multiset_.reserve(size);

      T key = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key);
        unordered_multiset_.insert(std::move(key));
      }
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::set<T>& set_)
    {
      _srz_impl_IDEBUGGING("std::set");

      size_t_serialization_implementation(set_.size());

      for (const auto& key : set_)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::set<T>& set_)
    {
      _srz_impl_IDEBUGGING("std::set");

      size_t size = {};
      size_t_deserialization_implementation(size);

      set_.clear();

      T key = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key);
        set_.insert(std::move(key));
      }
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::multiset<T>& multiset_)
    {
      _srz_impl_IDEBUGGING("std::multiset");

      size_t_serialization_implementation(multiset_.size());

      for (const auto& key : multiset_)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::multiset<T>& multiset_)
    {
      _srz_impl_IDEBUGGING("std::multiset");

      size_t size = {};
      size_t_deserialization_implementation(size);

      multiset_.clear();

      T key = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(key);
        multiset_.insert(std::move(key));
      }
    }

    template<size_t N, typename... T>
    struct _tuple_serialization
    {
      static inline constexpr
      void implementation(const std::tuple<T...>& tuple);
    };

    template<typename... T>
    struct _tuple_serialization<0, T...>
    {
      static inline constexpr
      void implementation(const std::tuple<T...>&) {}
    };

    template<size_t N, typename... T>
    constexpr
    void _tuple_serialization<N, T...>::implementation(const std::tuple<T...>& tuple_)
    {
      _serialization_implementation(std::get<sizeof...(T) - N>(tuple_));
      _tuple_serialization<N-1, T...>::implementation(tuple_);
    }

    template<typename... T>
    constexpr
    void _serialization_implementation(const std::tuple<T...>& tuple_)
    {
      _srz_impl_IDEBUGGING("std::tuple");

      _tuple_serialization<sizeof...(T), T...>::implementation(tuple_);
    }

    template<size_t N, typename... T>
    struct _tuple_deserialization
    {
      static inline constexpr
      void implementation(std::tuple<T...>& tuple);
    };

    template<typename... T>
    struct _tuple_deserialization<0, T...>
    {
      static inline constexpr
      void implementation(std::tuple<T...>&) {}
    };

    template<size_t N, typename... T>
    constexpr
    void _tuple_deserialization<N, T...>::implementation(std::tuple<T...>& tuple_)
    {
      _deserialization_implementation(std::get<sizeof...(T) - N>(tuple_));
      _tuple_deserialization<N-1, T...>::implementation(tuple_);
    }

    template<typename... T>
    constexpr
    void _deserialization_implementation(std::tuple<T...>& tuple_)
    {
      _srz_impl_IDEBUGGING("std::tuple");

      _tuple_deserialization<sizeof...(T), T...>::implementation(tuple_);
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  template<typename... T>
  _srz_impl_NODISCARD_REASON("serialize: ignoring the return value makes no sens")
  auto serialize(const T&... things_) noexcept -> std::vector<uint8_t>
  {
    _srz_impl_DEBUGGING("----serialization summary:");

    _impl::_buffer.clear();
    _impl::_buffer.reserve(_impl::_sizeof_things<T...>());

    _impl::_serialize_things(things_...);

    _srz_impl_DEBUGGING("----------------------------");

    return _impl::_buffer;
  }

  template<typename... T>
  Info deserialize(const uint8_t data_[], const size_t size_, T&... things_) noexcept
  {
    _srz_impl_DEBUGGING("----deserialization summary:");

    _impl::_buffer.assign(data_, data_ + size_);
    _impl::_front_of_buffer = 0;

    _srz_impl_SAFE(
      _impl::_info = Info::ALL_GOOD;
    )

    _impl::_deserialize_things(things_...);

    _srz_impl_SAFE(
      if _srz_impl_EXPECTED(_impl::_info == Info::ALL_GOOD)
      {
        if _srz_impl_ABNORMAL(_impl::_front_of_buffer != _impl::_buffer.size())
        {
          _impl::_info = Info::SEQUENCE_MISMATCH;
        }
      }
    )

    _srz_impl_DEBUGGING("----------------------------");

    return _impl::_info;
  }
//----------------------------------------------------------------------------------------------------------------------
  auto Serializable::serialize() const noexcept -> std::vector<uint8_t>
  {
    return srz::serialize(*this);
  }

  Info Serializable::deserialize(const uint8_t data_[], size_t size_) noexcept
  {
    return srz::deserialize(data_, size_, *this);
  }

  template<typename... T>
  void Serializable::serialization(const T&... data_) const noexcept
  {
    _impl::_serialize_things(data_...);
  }

  template<typename... T>
  void Serializable::deserialization(T&... data_) noexcept
  {
    _impl::_deserialize_things(data_...);
  }
//----------------------------------------------------------------------------------------------------------------------
# undef  SRZ_SERIALIZATION_SEQUENCE
# define SRZ_SERIALIZATION_SEQUENCE(...)                    \
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
  auto bytes_as_cstring(const uint8_t data[], const size_t size) -> const char*
  {
    _srz_impl_THREADLOCAL static std::vector<char> buffer;

    if (data == nullptr)
    {
      _srz_impl_DEBUGGING("data is nullptr");
      return nullptr;
    }

    if ((3*size + 1) > buffer.capacity())
    {
      buffer = std::vector<char>(3*size + 1);
    }

    buffer.clear();

    for (size_t k = 0; k < size; ++k)
    {
      const char nybl_hi = data[k] >> 4;
      const char nybl_lo = data[k] & 0xF;

      buffer.push_back((nybl_hi <= 0x9) ? (nybl_hi + '0') : (nybl_hi + 'A' - 10));
      buffer.push_back((nybl_lo <= 0x9) ? (nybl_lo + '0') : (nybl_lo + 'A' - 10));

      buffer.push_back(' ');
    }

    buffer[buffer.size() - 1] = '\0';

    return buffer.data();
  }
//----------------------------------------------------------------------------------------------------------------------
}
#undef _srz_impl_PRAGMA
#undef _srz_impl_CLANG_IGNORE
#undef _srz_impl_THREADLOCAL
#undef _srz_impl_ATOMIC
#undef _srz_impl_DECLARE_MUTEX
#undef _srz_impl_DECLARE_LOCK
#undef _srz_impl_LIKELY
#undef _srz_impl_UNLIKELY
#undef _srz_impl_EXPECTED
#undef _srz_impl_ABNORMAL
#undef _srz_impl_NODISCARD
#undef _srz_impl_MAYBE_UNUSED
#undef _srz_impl_NODISCARD_REASON
#undef _srz_impl_IDEBUGGING
#undef _srz_impl_DEBUGGING
#undef _srz_impl_CONSTEXPR_CPP14
#undef _srz_impl_CONSTEXPR_CPP17
#endif