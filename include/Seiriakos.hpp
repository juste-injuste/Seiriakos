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
  std::regex

  std::system_error
  std::hash ?? idk, im not sure about how it works
  std::basic_stringbuf ??
  std::basic_istringstream ??
  std::basic_ostringstream ??
  std::basic_stringstream ??
  std::locale ??
  std::ios ??
-----description------------------------------------------------------------------------------------

Seiriakos is a simple and lightweight C++11 (and newer) library that allows you serialize
and deserialize objects.

-----disclosure-------------------------------------------------------------------------------------

std::bitset is assumed to be contiguous.

std::priority_queue potentially triggers '-Wstrict-overflow' if compiling with GCC >= 9.1
with -Wstrict-overflow=3 and above.

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

#include <array>         // for std::array
#include <complex>       // for std::complex
#include <list>          // for std::list
#include <deque>         // for std::deque
#include <string>        // for std::basic_string
#include <utility>       // for std::pair
#include <unordered_map> // for std::unordered_map, std::unordered_multimap
#include <map>           // for std::map, std::multimap
#include <unordered_set> // for std::unordered_set, std::unordered_multiset
#include <set>           // for std::set, std::multiset
#include <tuple>         // for std::tuple
#include <valarray>      // for std::valarray
#include <bitset>        // for std::bitset
#include <atomic>        // for std::atomic
#include <chrono>        // for std::chrono::duration
#include <stack>         // for std::stack
#include <forward_list>  // for std::forward_list
#include <queue>         // for std::queue
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

  // macro to implement serialization/deserialization member functions
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

    constexpr
    operator Code() const noexcept
    {
      return _code;
    }

    constexpr
    const char* description() const
    {
      return "text";
    }

    operator bool() const = delete;

  private:
    Code _code;
  };
//----------------------------------------------------------------------------------------------------------------------
  namespace _impl
  {
#   define _srz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
# if defined(__clang__)
#   define _srz_impl_CLANG_IGNORE(WARNING, ...)          \
      _srz_impl_PRAGMA(clang diagnostic push)            \
      _srz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                        \
      _srz_impl_PRAGMA(clang diagnostic pop)
#   define _srz_impl_GCC_IGNORE(WARNING, ...)   __VA_ARGS__
# elif defined(__GNUC__)
#   define _srz_impl_CLANG_IGNORE(WARNING, ...) __VA_ARGS__
#   define _srz_impl_GCC_IGNORE(WARNING, ...)          \
      _srz_impl_PRAGMA(GCC diagnostic push)            \
      _srz_impl_PRAGMA(GCC diagnostic ignored WARNING) \
      __VA_ARGS__                                      \
      _srz_impl_PRAGMA(GCC diagnostic pop)
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
#   define _srz_impl_NODISCARD           __attribute__((warn_unused_result))
#   define _srz_impl_MAYBE_UNUSED        __attribute__((unused))
#   define _srz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _srz_impl_LIKELY
#   define _srz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _srz_impl_UNLIKELY
#   define _ltz_impl_RESTRICT            __restrict__
# elif defined(__GNUC__)
#   define _srz_impl_NODISCARD           __attribute__((warn_unused_result))
#   define _srz_impl_MAYBE_UNUSED        __attribute__((unused))
#   define _srz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _srz_impl_LIKELY
#   define _srz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _srz_impl_UNLIKELY
#   define _ltz_impl_RESTRICT            __restrict__
# else
#   define _srz_impl_NODISCARD
#   define _srz_impl_MAYBE_UNUSED
#   define _srz_impl_EXPECTED(CONDITION) (CONDITION) _srz_impl_LIKELY
#   define _srz_impl_ABNORMAL(CONDITION) (CONDITION) _srz_impl_UNLIKELY
#   define _ltz_impl_RESTRICT
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
          std::sprintf(_underlying_name_buffer, "uint%zu", sizeof(T) * 8);
        }
        else
        {
          std::sprintf(_underlying_name_buffer, "int%zu", sizeof(T) * 8);
        }
      }
      else if (std::is_floating_point<T>::value)
      {
        std::sprintf(_underlying_name_buffer, "float%zu", sizeof(T) * 8);
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
    _srz_impl_MAYBE_UNUSED static _srz_impl_THREADLOCAL char _dbg_buf[256] = {};

    class _indentdebug
    {
    public:
      template<typename... T>
      _srz_impl_CONSTEXPR_CPP14
      _indentdebug(T... arguments_) noexcept
      {
        _srz_impl_DECLARE_LOCK(_dbg_mtx);

        for (unsigned k = _depth()++; k; --k)
        {
          _io::dbg << "  ";
        }

        _srz_impl_GCC_IGNORE("-Wformat-security",   _srz_impl_CLANG_IGNORE("-Wformat-security",
        _srz_impl_GCC_IGNORE("-Wformat-nonliteral", _srz_impl_CLANG_IGNORE("-Wformat-nonliteral",
          std::sprintf(_dbg_buf, arguments_...);
        ))
        ))

        _io::dbg << _dbg_buf << std::endl;
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
#   define _srz_impl_DEBUGGING(...)                                  \
      [&](const char* const caller_){                                \
        _srz_impl_DECLARE_LOCK(_impl::_dbg_mtx);                     \
        std::sprintf(_impl::_dbg_buf, __VA_ARGS__);                  \
        _io::dbg << caller_ << ": " << _impl::_dbg_buf << std::endl; \
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
      _srz_impl_IDEBUGGING("%s x%zu", _underlying_name<T>(),  N_);

      const _ltz_impl_RESTRICT auto data_ptr = reinterpret_cast<const uint8_t*>(&data_);
      _buffer.insert(_buffer.end(), data_ptr, data_ptr + sizeof(T) * N_);
    }

    template<typename T, typename = _if_not_Serializable<T>>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(T& data_, const size_t N_ = 1)
    {
      _srz_impl_IDEBUGGING("%s x%zu", _underlying_name<T>(),  N_);

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

    template<typename T>
    void _serialization_implementation(const T* const data_) = delete;

    template<typename T>
    void _deserialization_implementation(T* const data_) = delete;

    _srz_impl_MAYBE_UNUSED
    static
    void _size_t_serialization_implementation(const size_t size_)
    {
#   if defined(SRZ_FIXED_LENGHT)
      _serialization_implementation(size_);
#   else
      _srz_impl_IDEBUGGING("size_t");

      uint8_t bytes_used = 1;
      for (size_t bytes = size_; bytes >>= 8; ++bytes_used) {}

      _buffer.push_back(bytes_used);

      for (size_t bytes = size_; bytes_used; bytes >>= 8, --bytes_used)
      {
        _buffer.push_back(static_cast<uint8_t>(bytes & 0xFF));
      }
#   endif
    }

    _srz_impl_MAYBE_UNUSED
    static
    void _size_t_deserialization_implementation(size_t& size_)
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

      size_ = {};
      for (size_t k = 0; bytes_used; k += 8, --bytes_used)
      {
        size_ |= (_buffer[_front_of_buffer++] << k);
      }
#   endif
    }

    inline
    void _serialization_implementation(const Serializable& serializable) noexcept;

    inline
    void _deserialization_implementation(Serializable& serializable) noexcept;

    template<typename T>
    constexpr
    void _serialization_implementation(const std::complex<T>& complex) noexcept;

    template<typename T>
    constexpr
    void _deserialization_implementation(std::complex<T>& complex) noexcept;

    template<typename T>
    constexpr
    void _serialization_implementation(const std::atomic<T>& atomic) noexcept;

    template<typename T>
    constexpr
    void _deserialization_implementation(std::atomic<T>& atomic) noexcept;

    template<class R, class P>
    constexpr
    void _serialization_implementation(const std::chrono::duration<R, P> duration) noexcept;

    template<class R, class P>
    constexpr
    void _deserialization_implementation(std::chrono::duration<R, P>& duration) noexcept;

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::basic_string<T>& string) noexcept;

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::basic_string<T>& string) noexcept;

    template<typename T, size_t N>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::array<T, N>& array) noexcept;

    template<typename T, size_t N>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::array<T, N>& array) noexcept;

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::vector<T>& vector) noexcept;

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::vector<T>& vector) noexcept;

    inline
    void _serialization_implementation(const std::vector<bool>& vector) noexcept;

    inline
    void _deserialization_implementation(std::vector<bool>& vector) noexcept;

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::valarray<T>& valarray) noexcept;

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::valarray<T>& valarray) noexcept;

    template<size_t N>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::bitset<N>& bitset) noexcept;

    template<size_t N>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::bitset<N>& bitset) noexcept;

    template<typename T>
    constexpr
    void _serialization_implementation(const std::list<T>& list) noexcept;

    template<typename T>
    constexpr
    void _deserialization_implementation(std::list<T>& list) noexcept;

    template<typename T>
    constexpr
    void _serialization_implementation(const std::stack<T>& stack) noexcept;

    template<typename T>
    constexpr
    void _deserialization_implementation(std::stack<T>& stack) noexcept;

    template<typename T>
    constexpr
    void _serialization_implementation(const std::forward_list<T>& forward_list) noexcept;

    template<typename T>
    constexpr
    void _deserialization_implementation(std::forward_list<T>& forward_list) noexcept;

    template<typename T, typename S>
    constexpr
    void _serialization_implementation(const std::queue<T, S>& queue) noexcept;

    template<typename T, typename S>
    constexpr
    void _deserialization_implementation(std::queue<T, S>& queue) noexcept;

    template<typename T, class C, class F>
    constexpr
    void _serialization_implementation(const std::priority_queue<T, C, F>& priority_queue) noexcept;

    template<typename T, class C, class F>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::priority_queue<T, C, F>& priority_queue) noexcept;

    template<typename T, typename A>
    constexpr
    void _serialization_implementation(const std::deque<T, A>& deque) noexcept;

    template<typename T, typename A>
    constexpr
    void _deserialization_implementation(std::deque<T, A>& deque) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::pair<T1, T2>& pair) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::pair<T1, T2>& pair) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::unordered_map<T1, T2>& unordered_map) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::unordered_map<T1, T2>& unordered_map) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::unordered_multimap<T1, T2>& unordered_multimap) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::unordered_multimap<T1, T2>& unordered_multimap) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::map<T1, T2>& map) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::map<T1, T2>& map) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::multimap<T1, T2>& multimap) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::multimap<T1, T2>& multimap) noexcept;

    template<typename T>
    constexpr
    void _serialization_implementation(const std::unordered_set<T>& unordered_set) noexcept;

    template<typename T>
    constexpr
    void _deserialization_implementation(std::unordered_set<T>& unordered_set) noexcept;

    template<typename T>
    constexpr
    void _serialization_implementation(const std::unordered_multiset<T>& unordered_multiset) noexcept;

    template<typename T>
    constexpr
    void _deserialization_implementation(std::unordered_multiset<T>& unordered_multiset) noexcept;

    template<typename T>
    constexpr
    void _serialization_implementation(const std::set<T>& set) noexcept;

    template<typename T>
    constexpr
    void _deserialization_implementation(std::set<T>& set) noexcept;

    template<typename T>
    constexpr
    void _serialization_implementation(const std::multiset<T>& multiset) noexcept;

    template<typename T>
    constexpr
    void _deserialization_implementation(std::multiset<T>& multiset) noexcept;

    template<typename... T>
    constexpr
    void _serialization_implementation(const std::tuple<T...>& tuple) noexcept;

    template<typename... T>
    constexpr
    void _deserialization_implementation(std::tuple<T...>& tuple) noexcept;

    template<typename T, typename... T_>
    constexpr
    auto _sizeof_things() noexcept -> typename std::enable_if<sizeof...(T_) == 0, size_t>::type
    {
      return sizeof(T);
    }

    template<typename T, typename... T_>
    constexpr
    auto _sizeof_things() noexcept -> typename std::enable_if<sizeof...(T_) != 0, size_t>::type
    {
      return sizeof(T) + _sizeof_things<T_...>();
    }

    inline _srz_impl_CONSTEXPR_CPP14 void _serialize_things() noexcept {}

    template<typename T, typename... T_>
    constexpr
    void _serialize_things(const T& thing, const T_&... remaining_things) noexcept
    {
      _serialization_implementation(thing);
      _serialize_things(remaining_things...);
    }

    inline _srz_impl_CONSTEXPR_CPP14 void _deserialize_things() noexcept {}

    template<typename T, typename... T_>
    constexpr
    void _deserialize_things(T& thing, T_&... remaining_things) noexcept
    {
      _deserialization_implementation(thing);
      _deserialize_things(remaining_things...);
    }

    namespace _serialization
    {
      template<typename... T>
      constexpr
      void serialization(const T&... data_) noexcept
      {
        _impl::_serialize_things(data_...);
      }
    }

    namespace _deserialization
    {
      template<typename... T>
      constexpr
      void serialization(T&... data_) noexcept
      {
        _impl::_deserialize_things(data_...);
      }
    }

    struct _Serializable_backdoor;
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
    virtual // serialization sequence (provided by the inheriting class via SRZ_SERIALIZATION_SEQUENCE)
    void serialization_sequence() const noexcept = 0;

    virtual // deserialization sequence (provided by the inheriting class via SRZ_SERIALIZATION_SEQUENCE)
    void deserialization_sequence() noexcept = 0;

    friend _impl::_Serializable_backdoor;
  };
//----------------------------------------------------------------------------------------------------------------------
  namespace _impl
  {
    struct _Serializable_backdoor
    {
      static _srz_impl_CONSTEXPR_CPP14
      void _serialization_sequence(const Serializable& serializable_) noexcept
      {
        serializable_.serialization_sequence();
      }

      static _srz_impl_CONSTEXPR_CPP14
      void _deserialization_sequence(Serializable& serializable_) noexcept
      {
        serializable_.deserialization_sequence();
      }
    };

    void _serialization_implementation(const Serializable& serializable_) noexcept
    {
      _srz_impl_IDEBUGGING("Serializable");

      _impl::_Serializable_backdoor::_serialization_sequence(serializable_);
    }

    void _deserialization_implementation(Serializable& serializable_) noexcept
    {
      _srz_impl_IDEBUGGING("Serializable");

      _impl::_Serializable_backdoor::_deserialization_sequence(serializable_);
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::complex<T>& complex_) noexcept
    {
      _srz_impl_IDEBUGGING("std::complex<%s>", _underlying_name<T>());

      _serialization_implementation(complex_.real);
      _serialization_implementation(complex_.imag);
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::complex<T>& complex_) noexcept
    {
      _srz_impl_IDEBUGGING("std::complex<%s>", _underlying_name<T>());

      _deserialization_implementation(complex_.real);
      _deserialization_implementation(complex_.imag);
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::atomic<T>& atomic_) noexcept
    {
      _srz_impl_IDEBUGGING("std::atomic<%s>", _underlying_name<T>());

      _serialization_implementation(static_cast<const T>(atomic_));
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::atomic<T>& atomic_) noexcept
    {
      _srz_impl_IDEBUGGING("std::atomic<%s>", _underlying_name<T>());

      T value = {};
      _deserialization_implementation(value);
      atomic_ = value;
    }

    template<class R, class P>
    constexpr
    void _serialization_implementation(const std::chrono::duration<R, P> duration_) noexcept
    {
      _srz_impl_IDEBUGGING("std::chrono::duration<%s, %s>", std::string(_underlying_name<R>()).c_str(), _underlying_name<P>());

      _serialization_implementation(duration_.count());
    }

    template<class R, class P>
    constexpr
    void _deserialization_implementation(std::chrono::duration<R, P>& duration_) noexcept
    {
      _srz_impl_IDEBUGGING("std::chrono::duration<%s, %s>", std::string(_underlying_name<R>()).c_str(), _underlying_name<P>());

      R count = {};
      _deserialization_implementation(duration_);
      duration_ = std::chrono::duration<R, P>(count);
    }

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::basic_string<T>& string_) noexcept
    {
      _srz_impl_IDEBUGGING("std::basic_string<%s>", _underlying_name<T>());

      _size_t_serialization_implementation(string_.size());

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
    void _deserialization_implementation(std::basic_string<T>& string_) noexcept
    {
      _srz_impl_IDEBUGGING("std::basic_string<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_deserialization_implementation(size);

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
    void _serialization_implementation(const std::array<T, N>& array_) noexcept
    {
      _srz_impl_IDEBUGGING("std::array<%s, %zu>", _underlying_name<T>(), N);

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
    void _deserialization_implementation(std::array<T, N>& array_) noexcept
    {
      _srz_impl_IDEBUGGING("std::array<%s, %zu>", _underlying_name<T>(), N);

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
    void _serialization_implementation(const std::vector<T>& vector_) noexcept
    {
      _srz_impl_IDEBUGGING("std::vector<%s>", _underlying_name<T>());

      _size_t_serialization_implementation(vector_.size());

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
    void _deserialization_implementation(std::vector<T>& vector_) noexcept
    {
      _srz_impl_IDEBUGGING("std::vector<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_deserialization_implementation(size);

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

    void _serialization_implementation(const std::vector<bool>& vector_) noexcept
    {
      _srz_impl_IDEBUGGING("std::vector<bool>");

      _size_t_serialization_implementation(vector_.size());

      for (const bool value : vector_)
      {
        _serialization_implementation(value);
      }
    }

    void _deserialization_implementation(std::vector<bool>& vector_) noexcept
    {
      _srz_impl_IDEBUGGING("std::vector<bool>");

      size_t size = {};
      _size_t_deserialization_implementation(size);

      vector_.resize(size);

      bool value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _deserialization_implementation(value);
        vector_.push_back(value);
      }
    }

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::valarray<T>& valarray_) noexcept
    {
      _srz_impl_IDEBUGGING("std::valarray<%s>", _underlying_name<T>());

      _size_t_serialization_implementation(valarray_.size());

      if _srz_impl_CONSTEXPR_CPP17 _srz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _serialization_implementation(valarray_[0], valarray_.size());
      }
      else
      {
        for (const auto& value : valarray_)
        {
          _serialization_implementation(value);
        }
      }
    }

    template<typename T>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::valarray<T>& valarray_) noexcept
    {
      _srz_impl_IDEBUGGING("std::valarray<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_deserialization_implementation(size);

      valarray_.resize(size);

      if _srz_impl_CONSTEXPR_CPP17 _srz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _deserialization_implementation(valarray_[0], size);
      }
      else
      {
        for (auto& value : valarray_)
        {
          _deserialization_implementation(value);
        }
      }
    }

    template<size_t N>
    _srz_impl_CONSTEXPR_CPP14
    void _serialization_implementation(const std::bitset<N>& bitset_) noexcept
    {
      _srz_impl_IDEBUGGING("std::bitset<%zu>", N);

      _serialization_implementation(bitset_, 1);
    }

    template<size_t N>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::bitset<N>& bitset_) noexcept
    {
      _srz_impl_IDEBUGGING("std::bitset<%zu>", N);

      _deserialization_implementation(bitset_, 1);
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::list<T>& list_) noexcept
    {
      _srz_impl_IDEBUGGING("std::list<%s>", _underlying_name<T>());

      _size_t_serialization_implementation(list_.size());

      for (const auto& value : list_)
      {
        _serialization_implementation(value);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::list<T>& list_) noexcept
    {
      _srz_impl_IDEBUGGING("std::list<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_deserialization_implementation(size);

      list_.resize(size);
      for (auto& value : list_)
      {
        _deserialization_implementation(value);
      }
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::stack<T>& stack_) noexcept
    {
      _srz_impl_IDEBUGGING("std::stack<%s>", _underlying_name<T>());

      std::stack<T> temp = stack_;

      const auto size = stack_.size();
      _size_t_serialization_implementation(size);

      for (size_t k = size; k; --k)
      {
        _serialization_implementation(temp.top());
        temp.pop();
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::stack<T>& stack_) noexcept
    {
      _srz_impl_IDEBUGGING("std::stack<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_deserialization_implementation(size);

      std::stack<T> temp;

      T value = {};
      for (size_t k = size; k; --k)
      {
        _deserialization_implementation(value);
        temp.push(std::move(value));
      }

      stack_ = std::stack<T>();
      for (size_t k = size; k; --k)
      {
        stack_.push(std::move(temp.top()));
        temp.pop();
      }
    }

    template<typename T>
    constexpr
    void _serialization_implementation(const std::forward_list<T>& forward_list_) noexcept
    {
      _srz_impl_IDEBUGGING("std::forward_list<%s>", _underlying_name<T>());

      size_t size = 0;
      for (auto iterator = forward_list_.begin(), end = forward_list_.end(); iterator != end; ++iterator)
      {
        ++size;
      }
      _size_t_serialization_implementation(size);

      for (const auto& value : forward_list_)
      {
        _serialization_implementation(value);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::forward_list<T>& forward_list_) noexcept
    {
      _srz_impl_IDEBUGGING("std::forward_list<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_deserialization_implementation(size);

      forward_list_.resize(size);
      for (auto& value : forward_list_)
      {
        _deserialization_implementation(value);
      }
    }

    template<typename T, typename S>
    constexpr
    void _serialization_implementation(const std::queue<T, S>& queue_) noexcept
    {
      _srz_impl_IDEBUGGING("std::queue<%s>", _underlying_name<T>());

      std::queue<T, S> temp = queue_;

      const auto size = queue_.size();
      _size_t_serialization_implementation(size);

      for (size_t k = size; k; --k)
      {
        _serialization_implementation(temp.front());
        temp.pop();
      }
    }

    template<typename T, typename S>
    constexpr
    void _deserialization_implementation(std::queue<T, S>& queue_) noexcept
    {
      _srz_impl_IDEBUGGING("std::queue<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_deserialization_implementation(size);

      queue_ = std::queue<T, S>();

      T value = {};
      for (size_t k = size; k; --k)
      {
        _deserialization_implementation(value);
        queue_.push(std::move(value));
      }
    }

    template<typename T, class C, class F>
    constexpr
    void _serialization_implementation(const std::priority_queue<T, C, F>& priority_queue_) noexcept
    {
      _srz_impl_IDEBUGGING("std::priority_queue<%s>", _underlying_name<std::priority_queue<T, C, F>>());

      std::priority_queue<T, C, F> temp = priority_queue_;

      const auto size = priority_queue_.size();

      _size_t_serialization_implementation(size);

      for (size_t k = size; k; --k)
      {
        _serialization_implementation(temp.top());
        temp.pop();
      }
    }

    template<typename T, class C, class F>
    _srz_impl_CONSTEXPR_CPP14
    void _deserialization_implementation(std::priority_queue<T, C, F>& priority_queue_) noexcept
    {
      _srz_impl_IDEBUGGING("std::priority_queue<%s>", _underlying_name<std::priority_queue<T, C, F>>());

      size_t size = 0;
      _size_t_deserialization_implementation(size);

      priority_queue_ = std::priority_queue<T, C, F>();

      T value = {};
      for (size_t k = size; k; --k)
      {
        _deserialization_implementation(value);
        priority_queue_.push(std::move(value));
      }
    }

    template<typename T, typename A>
    constexpr
    void _serialization_implementation(const std::deque<T, A>& deque_) noexcept
    {
      _srz_impl_IDEBUGGING("std::deque<%s>", _underlying_name<T>());

      _size_t_serialization_implementation(deque_.size());

      for (const auto& value : deque_)
      {
        _serialization_implementation(value);
      }
    }

    template<typename T, typename A>
    constexpr
    void _deserialization_implementation(std::deque<T, A>& deque_) noexcept
    {
      _srz_impl_IDEBUGGING("std::deque<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_deserialization_implementation(size);

      deque_.resize(size);
      for (auto& value : deque_)
      {
        _deserialization_implementation(value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::pair<T1, T2>& pair_) noexcept
    {
      _srz_impl_IDEBUGGING("std::pair<%s, %s>", _underlying_name<T1>(), _underlying_name<T2>());

      _serialization_implementation(pair_.first);
      _serialization_implementation(pair_.second);
    }

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::pair<T1, T2>& pair_) noexcept
    {
      _srz_impl_IDEBUGGING("std::pair<%s, %s>", _underlying_name<T1>(), _underlying_name<T2>());

      _deserialization_implementation(pair_.first);
      _deserialization_implementation(pair_.second);
    }

    template<typename T1, typename T2>
    constexpr
    void _serialization_implementation(const std::unordered_map<T1, T2>& unordered_map_) noexcept
    {
      _srz_impl_IDEBUGGING("std::unordered_map");

      _size_t_serialization_implementation(unordered_map_.size());

      for (const auto& key_value : unordered_map_)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::unordered_map<T1, T2>& unordered_map_) noexcept
    {
      _srz_impl_IDEBUGGING("std::unordered_map");

      size_t size = {};
      _size_t_deserialization_implementation(size);

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
    void _serialization_implementation(const std::unordered_multimap<T1, T2>& unordered_multimap_) noexcept
    {
      _srz_impl_IDEBUGGING("std::unordered_multimap");

      _size_t_serialization_implementation(unordered_multimap_.size());

      for (const auto& key_value : unordered_multimap_)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::unordered_multimap<T1, T2>& unordered_multimap_) noexcept
    {
      _srz_impl_IDEBUGGING("std::unordered_multimap");

      size_t size = {};
      _size_t_deserialization_implementation(size);

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
    void _serialization_implementation(const std::map<T1, T2>& map_) noexcept
    {
      _srz_impl_IDEBUGGING("std::map");

      _size_t_serialization_implementation(map_.size());

      for (const auto& key_value : map_)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::map<T1, T2>& map_) noexcept
    {
      _srz_impl_IDEBUGGING("std::map");

      size_t size = {};
      _size_t_deserialization_implementation(size);

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
    void _serialization_implementation(const std::multimap<T1, T2>& multimap_) noexcept
    {
      _srz_impl_IDEBUGGING("std::multimap");

      _size_t_serialization_implementation(multimap_.size());

      for (const auto& key_value : multimap_)
      {
        _serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _deserialization_implementation(std::multimap<T1, T2>& multimap_) noexcept
    {
      _srz_impl_IDEBUGGING("std::multimap");

      size_t size = {};
      _size_t_deserialization_implementation(size);

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
    void _serialization_implementation(const std::unordered_set<T>& unordered_set_) noexcept
    {
      _srz_impl_IDEBUGGING("std::unordered_set");

      _size_t_serialization_implementation(unordered_set_.size());

      for (const auto& key : unordered_set_)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::unordered_set<T>& unordered_set_) noexcept
    {
      _srz_impl_IDEBUGGING("std::unordered_set");

      size_t size = {};
      _size_t_deserialization_implementation(size);

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
    void _serialization_implementation(const std::unordered_multiset<T>& unordered_multiset_) noexcept
    {
      _srz_impl_IDEBUGGING("std::unordered_multiset");

      _size_t_serialization_implementation(unordered_multiset_.size());

      for (const auto& key : unordered_multiset_)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::unordered_multiset<T>& unordered_multiset_) noexcept
    {
      _srz_impl_IDEBUGGING("std::unordered_multiset");

      size_t size = {};
      _size_t_deserialization_implementation(size);

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
    void _serialization_implementation(const std::set<T>& set_) noexcept
    {
      _srz_impl_IDEBUGGING("std::set");

      _size_t_serialization_implementation(set_.size());

      for (const auto& key : set_)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::set<T>& set_) noexcept
    {
      _srz_impl_IDEBUGGING("std::set");

      size_t size = {};
      _size_t_deserialization_implementation(size);

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
    void _serialization_implementation(const std::multiset<T>& multiset_) noexcept
    {
      _srz_impl_IDEBUGGING("std::multiset");

      _size_t_serialization_implementation(multiset_.size());

      for (const auto& key : multiset_)
      {
        _serialization_implementation(key);
      }
    }

    template<typename T>
    constexpr
    void _deserialization_implementation(std::multiset<T>& multiset_) noexcept
    {
      _srz_impl_IDEBUGGING("std::multiset");

      size_t size = {};
      _size_t_deserialization_implementation(size);

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
      void implementation(const std::tuple<T...>& tuple) noexcept;
    };

    template<typename... T>
    struct _tuple_serialization<0, T...>
    {
      static inline constexpr
      void implementation(const std::tuple<T...>&) noexcept {}
    };

    template<size_t N, typename... T>
    constexpr
    void _tuple_serialization<N, T...>::implementation(const std::tuple<T...>& tuple_) noexcept
    {
      _serialization_implementation(std::get<sizeof...(T) - N>(tuple_));
      _tuple_serialization<N-1, T...>::implementation(tuple_);
    }

    template<typename... T>
    constexpr
    void _serialization_implementation(const std::tuple<T...>& tuple_) noexcept
    {
      _srz_impl_IDEBUGGING("std::tuple");

      _tuple_serialization<sizeof...(T), T...>::implementation(tuple_);
    }

    template<size_t N, typename... T>
    struct _tuple_deserialization
    {
      static inline constexpr
      void implementation(std::tuple<T...>& tuple) noexcept;
    };

    template<typename... T>
    struct _tuple_deserialization<0, T...>
    {
      static inline constexpr
      void implementation(std::tuple<T...>&) noexcept {}
    };

    template<size_t N, typename... T>
    constexpr
    void _tuple_deserialization<N, T...>::implementation(std::tuple<T...>& tuple_) noexcept
    {
      _deserialization_implementation(std::get<sizeof...(T) - N>(tuple_));
      _tuple_deserialization<N-1, T...>::implementation(tuple_);
    }

    template<typename... T>
    constexpr
    void _deserialization_implementation(std::tuple<T...>& tuple_) noexcept
    {
      _srz_impl_IDEBUGGING("std::tuple");

      _tuple_deserialization<sizeof...(T), T...>::implementation(tuple_);
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  template<typename... T>
  _srz_impl_NODISCARD_REASON("serialize: ignoring the return value makes no sens.")
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

  Info Serializable::deserialize(const uint8_t data_[], const size_t size_) noexcept
  {
    return srz::deserialize(data_, size_, *this);
  }
  
# undef  SRZ_SERIALIZATION_SEQUENCE
# define SRZ_SERIALIZATION_SEQUENCE(...)                    \
    private:                                                \
      void serialization_sequence() const noexcept override \
      {                                                     \
        using srz::_impl::_serialization::serialization;    \
        __VA_ARGS__                                         \
      }                                                     \
      void deserialization_sequence() noexcept override     \
      {                                                     \
        using srz::_impl::_deserialization::serialization;  \
        __VA_ARGS__                                         \
      }
      
# undef  SRZ_SERIALIZATION_TRIVIAL
# define SRZ_SERIALIZATION_TRIVIAL(...)                     \
    private:                                                \
      void serialization_sequence() const noexcept override \
      {                                                     \
        srz::_impl::_serialize_things(__VA_ARGS__);         \
      }                                                     \
      void deserialization_sequence() noexcept override     \
      {                                                     \
        srz::_impl::_deserialize_things(__VA_ARGS__);       \
      }
//----------------------------------------------------------------------------------------------------------------------
  auto bytes_as_cstring(const uint8_t data[], const size_t size) -> const char*
  {
    _srz_impl_THREADLOCAL static std::vector<char> buffer;

    if (data == nullptr)
    {
      _srz_impl_DEBUGGING("'data' is nullptr.");
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