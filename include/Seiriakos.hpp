/*---author-------------------------------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Seiriakos

-----licence------------------------------------------------------------------------------------------------------------

MIT License

Copyright (c) 2023 Justin Asselin (juste-injuste)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

-----versions-----------------------------------------------------------------------------------------------------------
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
-----description--------------------------------------------------------------------------------------------------------

Seiriakos is a simple and lightweight C++11 (and newer) library that allows you serialize
and deserialize base_ptrs.

-----disclosure---------------------------------------------------------------------------------------------------------

std::bitset is assumed to be contiguous.

std::priority_queue potentially triggers '-Wstrict-overflow' if compiling with GCC >= 9.1
with -Wstrict-overflow=3 and above.

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#ifndef _seiriakos_hpp
#define _seiriakos_hpp
#if __cplusplus >= 201103L
//---necessary standard libraries---------------------------------------------------------------------------------------
#include <cstddef>     // for size_t
#include <cstdint>     // for uint_fast8_t
#include <vector>      // for std::vector
#include <type_traits> // for std::enable_if, std::is_*, std::remove_pointer
#include <iostream>    // for std::clog
#include <cstring>     // for std::memcpy
//---conditionally necessary standard libraries-------------------------------------------------------------------------
#if defined(__STDCPP_THREADS__) and not defined(STZ_NOT_THREADSAFE)
# define  _stz_impl_THREADSAFE
# include <atomic> // for std::atomic
# include <mutex>  // for std::mutex, std::lock_guard
#endif
#if defined(STZ_DEBUGGING)
#if defined(__clang__) or defined(__GNUC__)
# include <cxxabi.h> // for abi::__cxa_demangle
#endif
# include <typeinfo> // for typeid
# include <cstdlib>  // for std::free
#endif
#if not defined(STZ_UNSAFE)
#include  <cassert>  // for assert
#endif
//*///------------------------------------------------------------------------------------------------------------------
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
//---Seiriakos library--------------------------------------------------------------------------------------------------
namespace stz
{
inline namespace seiriakos
//----------------------------------------------------------------------------------------------------------------------
{
  // macro to implement trivial serialization/deserialization
# define serialization_sequential(VARIABLES, ...)

  // macro to implement serialization/deserialization
# define serialization_procedural(...)

  // macro to add .serialize() and .deserialize(...) methods to a class
# define serialization_methods()

  enum Byte : uint8_t;

  using Bytes = std::vector<Byte>;

  // serialize 'things'
  template<typename... type>
  auto serialize(const type&... things) noexcept -> Bytes;

  // deserialize into 'things'
  template<typename... type>
  void deserialize(const Byte data[], size_t size, type&... things) noexcept;

  template<typename type>
  type deserialize(const Byte data[], size_t size) noexcept;
  
  template<class base, typename ptr>
  struct Inheritence;

  template<class base, class type>
  auto base_type(type* base_ptr) -> Inheritence<base, type*>;

  template<typename type>
  auto as_mutable(const type& const_data) -> type&;

  template<unsigned size, typename type>
  struct Bitfield;

  template<unsigned size, typename type>
  auto bitfield(type bitfield_data) -> Bitfield<size, type>;

  inline // convert bytes to const char*
  auto hex_string(const Byte data[], const size_t size) -> const char*;

  namespace io
  {
    static std::ostream out(std::cout.rdbuf()); // output
    static std::ostream log(std::clog.rdbuf()); // logging
    static std::ostream dbg(std::clog.rdbuf()); // debugging
    static std::ostream wrn(std::cerr.rdbuf()); // warnings
    static std::ostream err(std::cerr.rdbuf()); // errors
  }

# define SEIRIAKOS_MAJOR   000
# define SEIRIAKOS_MINOR   000
# define SEIRIAKOS_PATCH   000
# define SEIRIAKOS_VERSION ((SEIRIAKOS_MAJOR  * 1000 + SEIRIAKOS_MINOR) * 1000 + SEIRIAKOS_PATCH)
//*///------------------------------------------------------------------------------------------------------------------
  template<class base, typename ptr>
  struct Inheritence
  {
    static_assert(std::is_base_of<base, typename std::remove_pointer<ptr>::type>::value,
      "stz: Inheritence: 'ptr' must be a pointer to a type derived from 'base'."
    );

    const ptr base_ptr;
  };

  template<class base, class type>
  auto base_type(type* const base_ptr_) -> Inheritence<base, type*>
  {
    return Inheritence<base, type*>{base_ptr_};
  }

  template<typename type>
  auto as_mutable(const type& const_data_) -> type&
  {
    return const_cast<type&>(const_data_);
  }

  template<unsigned size, typename type>
  struct Bitfield
  {
    type proxy = {};
  };
//*///------------------------------------------------------------------------------------------------------------------
  namespace _seiriakos_impl
  {
#   define _stz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
# if defined(__clang__)
#   define _stz_impl_CLANG_IGNORE(WARNING, ...)          \
      _stz_impl_PRAGMA(clang diagnostic push)            \
      _stz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                        \
      _stz_impl_PRAGMA(clang diagnostic pop)

#   define _stz_impl_GCC_IGNORE(WARNING, ...)   __VA_ARGS__

#   define _stz_impl_GCC_CLANG_IGNORE(WARNING, ...) _stz_impl_CLANG_IGNORE(WARNING, __VA_ARGS__)
# elif defined(__GNUC__)
#   define _stz_impl_CLANG_IGNORE(WARNING, ...) __VA_ARGS__

#   define _stz_impl_GCC_IGNORE(WARNING, ...)          \
      _stz_impl_PRAGMA(GCC diagnostic push)            \
      _stz_impl_PRAGMA(GCC diagnostic ignored WARNING) \
      __VA_ARGS__                                      \
      _stz_impl_PRAGMA(GCC diagnostic pop)

#   define _stz_impl_GCC_CLANG_IGNORE(WARNING, ...) _stz_impl_GCC_IGNORE(WARNING, __VA_ARGS__)
# else
#   define _stz_impl_CLANG_IGNORE(WARNING, ...)     __VA_ARGS__
#   define _stz_impl_GCC_IGNORE(WARNING, ...)       __VA_ARGS__
#   define _stz_impl_GCC_CLANG_IGNORE(WARNING, ...) __VA_ARGS__
#endif

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define _stz_impl_LIKELY   _stz_impl_CLANG_IGNORE("-Wc++20-extensions", [[likely]])
#   define _stz_impl_UNLIKELY _stz_impl_CLANG_IGNORE("-Wc++20-extensions", [[unlikely]])
# else
#   define _stz_impl_LIKELY   [[likely]]
#   define _stz_impl_UNLIKELY [[unlikely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _stz_impl_LIKELY   [[likely]]
#   define _stz_impl_UNLIKELY [[unlikely]]
# else
#   define _stz_impl_LIKELY
#   define _stz_impl_UNLIKELY
# endif

// support from clang 3.9.0 and GCC 4.7.3 onward
# if defined(__clang__)
#   define _stz_impl_NODISCARD           __attribute__((warn_unused_result))
#   define _stz_impl_MAYBE_UNUSED        __attribute__((unused))
#   define _stz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _stz_impl_LIKELY
#   define _stz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _stz_impl_UNLIKELY
#   define _ltz_impl_RESTRICT            __restrict__
# elif defined(__GNUC__)
#   define _stz_impl_NODISCARD           __attribute__((warn_unused_result))
#   define _stz_impl_MAYBE_UNUSED        __attribute__((unused))
#   define _stz_impl_EXPECTED(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 1)) _stz_impl_LIKELY
#   define _stz_impl_ABNORMAL(CONDITION) (__builtin_expect(static_cast<bool>(CONDITION), 0)) _stz_impl_UNLIKELY
#   define _ltz_impl_RESTRICT            __restrict__
# else
#   define _stz_impl_NODISCARD
#   define _stz_impl_MAYBE_UNUSED
#   define _stz_impl_EXPECTED(CONDITION) (CONDITION) _stz_impl_LIKELY
#   define _stz_impl_ABNORMAL(CONDITION) (CONDITION) _stz_impl_UNLIKELY
#   define _ltz_impl_RESTRICT
# endif

// support from clang 10.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 10)
# if __cplusplus < 202002L
#   define _stz_impl_NODISCARD_REASON(REASON) _stz_impl_CLANG_IGNORE("-Wc++20-extensions", [[nodiscard(REASON)]])
# else
#   define _stz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _stz_impl_NODISCARD_REASON(REASON) [[nodiscard(REASON)]]
# else
#   define _stz_impl_NODISCARD_REASON(REASON) _stz_impl_NODISCARD
# endif

# if __cplusplus >= 201402L
#   define _stz_impl_CONSTEXPR_CPP14 constexpr
# else
#   define _stz_impl_CONSTEXPR_CPP14
# endif

# if __cplusplus >= 201703L
#   define _stz_impl_CONSTEXPR_CPP17 constexpr
# else
#   define _stz_impl_CONSTEXPR_CPP17
# endif

# if defined(_stz_impl_THREADSAFE)
#   undef  _stz_impl_THREADSAFE
#   define _stz_impl_THREADLOCAL         thread_local
#   define _stz_impl_ATOMIC(T)           std::atomic<T>
#   define _stz_impl_DECLARE_MUTEX(...)  static std::mutex __VA_ARGS__
#   define _stz_impl_DECLARE_LOCK(MUTEX) std::lock_guard<decltype(MUTEX)> _lock{MUTEX}
# else
#   define _stz_impl_THREADLOCAL
#   define _stz_impl_ATOMIC(T)           T
#   define _stz_impl_DECLARE_MUTEX(...)
#   define _stz_impl_DECLARE_LOCK(MUTEX)
# endif

    static _stz_impl_THREADLOCAL Bytes  _buffer;
    static _stz_impl_THREADLOCAL size_t _buffer_front;

# if defined(STZ_DEBUGGING)
    template<typename T>
    auto _underlying_name() -> const char*
    {
      static _stz_impl_THREADLOCAL char _underlying_name_buffer[256];

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
        static _stz_impl_THREADLOCAL size_t size = sizeof(_underlying_name_buffer);
        abi::__cxa_demangle(typeid(T).name(), _underlying_name_buffer, &size, nullptr);
#   else
        std::sprintf(_underlying_name_buffer, "%s", typeid(T).name());
#   endif
      }

      return _underlying_name_buffer;
    }

    _stz_impl_DECLARE_MUTEX(_dbg_mtx);
    _stz_impl_MAYBE_UNUSED static _stz_impl_THREADLOCAL char _dbg_buf[256] = {};

    class _indentdebug
    {
    public:
      template<typename... T>
      _stz_impl_CONSTEXPR_CPP14
      _indentdebug(T... arguments_) noexcept
      {
        _stz_impl_DECLARE_LOCK(_dbg_mtx);

        for (unsigned k = _depth()++; k; --k)
        {
          io::dbg << "  ";
        }

        _stz_impl_GCC_CLANG_IGNORE("-Wformat-security", _stz_impl_GCC_CLANG_IGNORE("-Wformat-nonliteral",
          std::sprintf(_dbg_buf, arguments_...);
        ))

        io::dbg << _dbg_buf << std::endl;
      }

      ~_indentdebug() noexcept { --_depth(); }
    private:
      _stz_impl_ATOMIC(unsigned)& _depth()
      {
        static _stz_impl_ATOMIC(unsigned) indentation = {0};
        return indentation;
      };
    };

#   define _stz_impl_IDEBUGGING(...) _seiriakos_impl::_indentdebug _idbg(__VA_ARGS__)
#   define _stz_impl_DEBUGGING(...)                                  \
      [&](const char* const caller_){                                \
        _stz_impl_DECLARE_LOCK(_impl::_dbg_mtx);                     \
        std::sprintf(_seiriakos_impl::_dbg_buf, __VA_ARGS__);                  \
        io::dbg << caller_ << ": " << _seiriakos_impl::_dbg_buf << std::endl; \
      }(__func__)
# else
#   define _stz_impl_IDEBUGGING(...) void(0)
#   define _stz_impl_DEBUGGING(...)  void(0)
# endif

    _stz_impl_DECLARE_MUTEX(_wrn_mtx);
    _stz_impl_MAYBE_UNUSED static _stz_impl_THREADLOCAL char _wrn_buf[256] = {};

#   define _stz_impl_WARNING(...)                                             \
      [&](const char* const caller_){                                         \
        _stz_impl_DECLARE_LOCK(_seiriakos_impl::_wrn_mtx);                    \
        std::sprintf(_seiriakos_impl::_wrn_buf, __VA_ARGS__);                 \
        io::wrn << caller_ << ": " << _seiriakos_impl::_wrn_buf << std::endl; \
      }(__func__)

# if defined(STZ_UNSAFE)
#   define _stz_impl_SAFE(...)
#   define _stz_impl_UNSAFE(...) __VA_ARGS__
# else
#   define _stz_impl_SAFE(...)   __VA_ARGS__
#   define _stz_impl_UNSAFE(...)
#endif

    constexpr size_t _next(size_t k, size_t N) { return k < N ? k + 1 : N; }

    constexpr
    bool _is_allowed(const char character)
    {
      return (('a' <= character) && (character<= 'z'))
        ||   (('A' <= character) && (character<= 'Z'))
        ||   (('0' <= character) && (character<= '1'))
        ||   (character == ' ')
        ||   (character == '.')
        ||   (character == ',')
        ||   (character == '\0');
    }

    template<size_t N1, size_t k1 = 0>
    constexpr
    bool _is_variable_list(const char (&string)[N1])
    {
      // return (k >= N) ? true                           // end was reached successfully
      //   : _is_allowed(string[k])                       // check current character
      //   and _is_variable_list<N, _next(k, N)>(string); // check next character if previous was correct using short-circuiting
      return true;
    }

    template<size_t N, size_t k = 0>
    constexpr
    bool _has(const char (&string)[N], const char character)
    {
      return (k >= N) ? false : string[k] == character || _has<N, _next(k, N)>(string, character);
    }
      
#   define _stz_impl_assert_trivial(STRING)                     \
      constexpr const char string[] = STRING;                   \
      static_assert(                                            \
        stz::_seiriakos_impl::_is_variable_list(string),        \
        "stz: trivial_serialization: must only list variables." \
      )

    struct _backdoor final
    {
    private:
      template<typename T_>
      static
      auto _has_seq_impl(int) -> decltype
      (
        void(std::declval<T_&>()._stz_impl_srz_seq()),
        void(std::declval<T_&>()._stz_impl_drz_seq()),
        std::true_type()
      );

      template<typename T_>
      static
      auto _has_seq_impl(...) -> std::false_type;

    public:
      template<typename T>
      static constexpr
      bool _has_seq()
      {
        return decltype(_has_seq_impl<T>(0))();
      }

      template<typename T>
      static _stz_impl_CONSTEXPR_CPP14
      void _srz_seq(const T& serializable_) noexcept
      {
        serializable_._stz_impl_srz_seq();
      }

      template<typename T>
      static _stz_impl_CONSTEXPR_CPP14
      void _drz_seq(T& serializable_) noexcept
      {
        serializable_._stz_impl_drz_seq();
      }

      template<typename base, typename ptr>
      static
      void _srz_impl_on_base(const Inheritence<base, ptr>&& inheritence_);

      template<typename base, typename ptr>
      static
      void _drz_impl_on_base(Inheritence<base, ptr>&& inheritence_);
    };
    
    template<typename type>
    constexpr
    void _srz_impl(const type&&)
    {
      static_assert(false, "stz: serialization: cannot serialize rvalues.");
    };

    template<typename type>
    constexpr
    void _drz_impl(type&&)
    {
      static_assert(false, "stz: deserialization: cannot deserialize into rvalues.");
    };
    
    template<typename base, typename ptr>
    void _srz_impl(const Inheritence<base, ptr>&) noexcept = delete;

    template<typename base, typename ptr>
    void _drz_impl(Inheritence<base, ptr>&) = delete;
    
    template<unsigned size, typename type>
    void _srz_impl(const Bitfield<size, type>&) noexcept = delete;

    template<unsigned size, typename type>
    void _drz_impl(Bitfield<size, type>&) noexcept = delete;

    // template<typename T>
    // void _srz_impl(const T* const data_)
    // {
    //   _stz_impl_IDEBUGGING("pointer to:");

    //   _srz_impl(*data_);
    // }

    // template<typename T>
    // void _drz_impl(T* const data_)
    // {
    //   _stz_impl_IDEBUGGING("pointer to:");

    //   // if (data_ == nullptr) data_ = new T;

    //   _drz_impl(*data_);
    // }

    template<typename T>
    using _if_sequence = typename std::enable_if<_backdoor::_has_seq<T>() == true>::type;

    template<typename T>
    using _no_sequence = typename std::enable_if<_backdoor::_has_seq<T>() != true>::type;

    template<typename T, typename = _if_sequence<T>>
    constexpr
    void _srz_impl(const T& serializable_) noexcept
    {
      _stz_impl_IDEBUGGING("%s", _underlying_name<T>());

      _seiriakos_impl::_backdoor::_srz_seq(serializable_);
    }

    template<typename T, typename = _if_sequence<T>>
    constexpr
    void _drz_impl(T& serializable_) noexcept
    {
      _stz_impl_IDEBUGGING("%s", _underlying_name<T>());

      _seiriakos_impl::_backdoor::_drz_seq(serializable_);
    }

    template<typename T, typename = _no_sequence<T>>
    constexpr
    void _srz_impl(const T& data_, const size_t N_ = 1)
    {
      if (N_ > 1) _stz_impl_IDEBUGGING("%s x%zu", _underlying_name<T>(),  N_);
      else        _stz_impl_IDEBUGGING("%s",      _underlying_name<T>());

      const _ltz_impl_RESTRICT auto data_ptr = reinterpret_cast<const Byte*>(&data_);

      _buffer.insert(_buffer.end(), data_ptr, data_ptr + sizeof(T) * N_);
    }

    template<typename T, typename = _no_sequence<T>>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(T& data_, const size_t N_ = 1)
    {
      if (N_ > 1) _stz_impl_IDEBUGGING("%s x%zu", _underlying_name<T>(),  N_);
      else        _stz_impl_IDEBUGGING("%s",      _underlying_name<T>());

      _stz_impl_SAFE(
      if _stz_impl_ABNORMAL(_buffer_front >= _buffer.size())
      {
        return;
      }

      if _stz_impl_ABNORMAL((_buffer.size() - _buffer_front) < (sizeof(T) * N_))
      {
        return;
      })

      // set data's bytes one by one from the front of the buffer
      const auto data_ptr   = reinterpret_cast<Byte*>(&data_);
      const auto buffer_ptr = _buffer.data() + _buffer_front;
      std::memcpy(data_ptr, buffer_ptr, sizeof(T) * N_);

      _buffer_front += sizeof(T) * N_;
    }
    
    template<typename base, typename ptr>
    constexpr
    void _srz_impl(const Inheritence<base, ptr>&& inheritence_) noexcept
    {
      _backdoor::_srz_impl_on_base(std::move(inheritence_));
    }

    template<typename base, typename ptr>
    constexpr
    void _drz_impl(Inheritence<base, ptr>&& inheritence_) noexcept
    {
      _backdoor::_drz_impl_on_base(std::move(inheritence_));
    }
    
    template<unsigned size, typename type>
    constexpr
    void _srz_impl(const Bitfield<size, type>&& bitfield_) noexcept
    {
      _srz_impl(bitfield_.proxy);
    }

    template<unsigned size, typename type>
    constexpr
    void _drz_impl(Bitfield<size, type>&& bitfield_) noexcept
    {
      _drz_impl(bitfield_.proxy);
    }

    template<typename T, size_t N1>
    void _srz_impl(const T (&data_)[N1])
    {
      _srz_impl(*static_cast<const T*>(data_), N1);
    }

    template<typename T, size_t N1>
    void _drz_impl(T (&data_)[N1])
    {
      _drz_impl(*static_cast<T*>(data_), N1);
    }

    _stz_impl_MAYBE_UNUSED
    static
    void _size_t_srz_impl(const size_t size_)
    {
#   if defined(STZ_FIXED_SERIALIZATION)
      _srz_impl(size_);
#   else
      _stz_impl_IDEBUGGING("size_t");

      uint8_t bytes_used = 1;
      for (size_t bytes = size_; bytes >>= 8; ++bytes_used) {}

      _buffer.push_back(static_cast<Byte>(bytes_used));

      for (size_t bytes = size_; bytes_used; bytes >>= 8, --bytes_used)
      {
        _buffer.push_back(static_cast<Byte>(bytes & 0xFF));
      }
#   endif
    }

    _stz_impl_MAYBE_UNUSED
    static
    void _size_t_drz_impl(size_t& size_)
    {
#   if defined(STZ_FIXED_SERIALIZATION)
      _drz_impl(size_);
#   else
      _stz_impl_IDEBUGGING("size_t");

      _stz_impl_SAFE(
      if _stz_impl_ABNORMAL(_buffer_front >= _buffer.size())
      {
        return;
      })

      uint8_t bytes_used = _buffer[_buffer_front++];

      _stz_impl_SAFE(
      if _stz_impl_ABNORMAL((_buffer.size() - _buffer_front) < bytes_used)
      {
        return;
      })

      size_ = {};
      for (size_t k = 0; bytes_used; k += 8, --bytes_used)
      {
        size_ |= (_buffer[_buffer_front++] << k);
      }
#   endif
    }

    template<typename T>
    constexpr
    void _srz_impl(const std::complex<T>& complex) noexcept;

    template<typename T>
    constexpr
    void _drz_impl(std::complex<T>& complex) noexcept;

    template<typename T>
    constexpr
    void _srz_impl(const std::atomic<T>& atomic) noexcept;

    template<typename T>
    constexpr
    void _drz_impl(std::atomic<T>& atomic) noexcept;

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl(const std::basic_string<T>& string) noexcept;

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::basic_string<T>& string) noexcept;

    template<typename T, size_t N1>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl(const std::array<T, N1>& array) noexcept;

    template<typename T, size_t N1>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::array<T, N1>& array) noexcept;

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl(const std::vector<T>& vector) noexcept;

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::vector<T>& vector) noexcept;

    inline
    void _srz_impl(const std::vector<bool>& vector) noexcept;

    inline
    void _drz_impl(std::vector<bool>& vector) noexcept;

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl(const std::valarray<T>& valarray) noexcept;

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::valarray<T>& valarray) noexcept;

    template<size_t N1>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl(const std::bitset<N1>& bitset) noexcept;

    template<size_t N1>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::bitset<N1>& bitset) noexcept;

    template<typename T>
    constexpr
    void _srz_impl(const std::list<T>& list) noexcept;

    template<typename T>
    constexpr
    void _drz_impl(std::list<T>& list) noexcept;

    template<typename T>
    constexpr
    void _srz_impl(const std::stack<T>& stack) noexcept;

    template<typename T>
    constexpr
    void _drz_impl(std::stack<T>& stack) noexcept;

    template<typename T>
    constexpr
    void _srz_impl(const std::forward_list<T>& forward_list) noexcept;

    template<typename T>
    constexpr
    void _drz_impl(std::forward_list<T>& forward_list) noexcept;

    template<typename T, typename S>
    constexpr
    void _srz_impl(const std::queue<T, S>& queue) noexcept;

    template<typename T, typename S>
    constexpr
    void _drz_impl(std::queue<T, S>& queue) noexcept;

    template<typename T, class C, class F>
    constexpr
    void _srz_impl(const std::priority_queue<T, C, F>& priority_queue) noexcept;

    template<typename T, class C, class F>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::priority_queue<T, C, F>& priority_queue) noexcept;

    template<typename T, typename A>
    constexpr
    void _srz_impl(const std::deque<T, A>& deque) noexcept;

    template<typename T, typename A>
    constexpr
    void _drz_impl(std::deque<T, A>& deque) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _srz_impl(const std::pair<T1, T2>& pair) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _drz_impl(std::pair<T1, T2>& pair) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _srz_impl(const std::unordered_map<T1, T2>& unordered_map) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _drz_impl(std::unordered_map<T1, T2>& unordered_map) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _srz_impl(const std::unordered_multimap<T1, T2>& unordered_multimap) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _drz_impl(std::unordered_multimap<T1, T2>& unordered_multimap) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _srz_impl(const std::map<T1, T2>& map) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _drz_impl(std::map<T1, T2>& map) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _srz_impl(const std::multimap<T1, T2>& multimap) noexcept;

    template<typename T1, typename T2>
    constexpr
    void _drz_impl(std::multimap<T1, T2>& multimap) noexcept;

    template<typename T>
    constexpr
    void _srz_impl(const std::unordered_set<T>& unordered_set) noexcept;

    template<typename T>
    constexpr
    void _drz_impl(std::unordered_set<T>& unordered_set) noexcept;

    template<typename T>
    constexpr
    void _srz_impl(const std::unordered_multiset<T>& unordered_multiset) noexcept;

    template<typename T>
    constexpr
    void _drz_impl(std::unordered_multiset<T>& unordered_multiset) noexcept;

    template<typename T>
    constexpr
    void _srz_impl(const std::set<T>& set) noexcept;

    template<typename T>
    constexpr
    void _drz_impl(std::set<T>& set) noexcept;

    template<typename T>
    constexpr
    void _srz_impl(const std::multiset<T>& multiset) noexcept;

    template<typename T>
    constexpr
    void _drz_impl(std::multiset<T>& multiset) noexcept;

    template<typename... T>
    constexpr
    void _srz_impl(const std::tuple<T...>& tuple) noexcept;

    template<typename... T>
    constexpr
    void _drz_impl(std::tuple<T...>& tuple) noexcept;

    template<typename T>
    using _if_fundamental = typename std::enable_if<std::is_fundamental<T>::value == true>::type;

    template<typename T>
    using _no_fundamental = typename std::enable_if<std::is_fundamental<T>::value != true>::type;

    template<typename T, typename = _if_fundamental<T>>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl_many_fundamentals(const T& things_, const size_t count_) noexcept
    {
      _srz_impl(things_, count_);
    }

    template<typename T, typename = _no_fundamental<T>>
    constexpr
    void _srz_impl_many_fundamentals(const T&, const size_t, ...) noexcept
    {}

    template<typename T, typename = _if_fundamental<T>>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl_many_fundamentals(T& things_, const size_t count_) noexcept
    {
      _drz_impl(things_, count_);
    }

    template<typename T, typename = _no_fundamental<T>>
    constexpr
    void _drz_impl_many_fundamentals(T&, const size_t, ...) noexcept
    {}

    template<typename base, typename ptr>
    void _backdoor::_srz_impl_on_base(const Inheritence<base, ptr>&& inheritence_)
    {
      _srz_impl(*static_cast<const base*>(inheritence_.base_ptr));
    }

    template<typename base, typename ptr>
    void _backdoor::_drz_impl_on_base(Inheritence<base, ptr>&& inheritence_)
    {
      _drz_impl(*static_cast<base*>(inheritence_.base_ptr));
    }

    template<typename T, typename... T_>
    constexpr
    auto _sizeof_many() noexcept -> typename std::enable_if<sizeof...(T_) == 0, size_t>::type
    {
      return sizeof(T);
    }

    template<typename T, typename... T_>
    constexpr
    auto _sizeof_many() noexcept -> typename std::enable_if<sizeof...(T_) != 0, size_t>::type
    {
      return sizeof(T) + _sizeof_many<T_...>();
    }

    template<typename T>
    using _strip = typename std::remove_reference<T>::type;

    // Helper to check if a single type is non-const
    template<typename T>
    struct _is_mutable : std::integral_constant<bool, not std::is_const<_strip<T>>::value>
    {};

    // Recursive template to check if all types are non-const
    template<typename... T_>
    struct _all_mutable;

    template<typename T, typename... T_>
    struct _all_mutable<T, T_...> : std::integral_constant<bool, _is_mutable<T>::value && _all_mutable<T_...>::value>
    {};

    template<>
    struct _all_mutable<> : std::true_type
    {};

    // _if_mutable type alias
    template<typename... T_>
    using _if_mutable = typename std::enable_if<_all_mutable<T_...>::value>::type;

    constexpr int _srz_dispatch() noexcept { return 0; }

    template<typename T, typename... T_>
    constexpr
    void _srz_dispatch(T&& thing_, T_&&... things_) noexcept
    {
      _srz_impl(std::forward<const T>(thing_));
      _srz_dispatch(std::forward<const T_>(things_)...);
    }

    constexpr int _drz_dispatch() noexcept { return 0; }

    template<typename T, typename... T_, typename = _if_mutable<T, T_...>>
    constexpr
    void _drz_dispatch(T&& thing_, T_&&... things_) noexcept
    {
      _drz_impl(std::forward<T>(thing_));
      _drz_dispatch(std::forward<T_>(things_)...);
    }

    struct _srz
    {
      struct _ver
      {
        template<typename type>
        void operator=(const type version_) noexcept
        {
          _version = static_cast<size_t>(version_);

          _stz_impl_SAFE(
          if (version_ < static_cast<type>(0))
          {
            _stz_impl_WARNING("invalid 'serializer.version' value (%zu).", _version);
            _version = static_cast<type>(-1);
          } else)
          {
            _size_t_srz_impl(_version);
          }
        }

        _stz_impl_UNSAFE(constexpr)
        bool operator<(size_t) const noexcept
        {
          _stz_impl_SAFE(
          if (_version == static_cast<size_t>(-1))
          {
            _stz_impl_WARNING("'serializer.version' was not set properly before usage.");
          })

          return false;
        }

        _stz_impl_UNSAFE(constexpr)
        bool operator<=(size_t) const noexcept
        {
          _stz_impl_SAFE(
          if (_version == static_cast<size_t>(-1))
          {
            _stz_impl_WARNING("'serializer.version' was not set properly before usage.");
          })

          return false;
        }

        _stz_impl_UNSAFE(constexpr)
        bool operator>(size_t) const noexcept
        {
          _stz_impl_SAFE(
          if (_version == static_cast<size_t>(-1))
          {
            _stz_impl_WARNING("'serializer.version' was not set properly before usage.");
          })

          return true;
        }

        _stz_impl_UNSAFE(constexpr)
        bool operator>=(size_t) const noexcept
        {
          _stz_impl_SAFE(
          if (_version == static_cast<size_t>(-1))
          {
            _stz_impl_WARNING("'serializer.version' was not set properly before usage.");
            std::exit(-1);
          })

          return true;
        }

      private:
        size_t _version = static_cast<size_t>(-1);
      };

      void operator<=(_ver) const = delete;
      void operator, (_ver) const = delete;

      _ver version;

      template<typename type>
      constexpr
      _srz operator<=(const type& thing_) const &
      {
        return _srz_impl(thing_), _srz();
      }

      template<typename type>
      constexpr
      _srz operator,(const type& thing_) const &&
      {
        return _srz_impl(thing_), _srz();
      }

      template<typename type>
      void operator,(const type) const & = delete;
    };

    struct _drz
    {
      struct _ver
      {
        template<typename type>
        void operator=(type _stz_impl_SAFE(version_))
        {
          _stz_impl_SAFE(
          if (version_ >= static_cast<type>(0))
        ) {
            _size_t_drz_impl(_version);
          }
        }

        constexpr bool operator< (const size_t version_) const noexcept { return _version <  version_; }
        constexpr bool operator<=(const size_t version_) const noexcept { return _version <= version_; }
        constexpr bool operator> (const size_t version_) const noexcept { return _version >  version_; }
        constexpr bool operator>=(const size_t version_) const noexcept { return _version >= version_; }

      private:
        size_t _version = static_cast<size_t>(-1);
      };

      void operator<=(_ver) const = delete;
      void operator, (_ver) const = delete;

      _ver version;

      template<typename type>
      constexpr
      _drz operator<=(type& thing_) const &
      {
        return _drz_impl(thing_), _drz();
      }

      template<typename type>
      constexpr
      _drz operator,(type& thing_) const &&
      {
        return _drz_impl(thing_), _drz();
      }

      template<typename type>
      void operator<=(const type&) const & = delete;

      template<typename type>
      void operator,(const type&) const && = delete;

      template<class base, typename ptr>
      constexpr
      _drz operator<=(Inheritence<base, ptr>&& thing_) const &
      {
        return _drz_impl(thing_), _drz();
      }

      template<class base, typename ptr>
      constexpr
      _drz operator,(Inheritence<base, ptr>&& thing_) const &&
      {
        return _drz_impl(thing_), _drz();
      }

      template<typename type>
      void operator,(type) const & = delete;
    };

    template<typename T>
    constexpr
    void _srz_impl(const std::complex<T>& complex_) noexcept
    {
      _stz_impl_IDEBUGGING("std::complex<%s>", _underlying_name<T>());

      _srz_impl(complex_.real);
      _srz_impl(complex_.imag);
    }

    template<typename T>
    constexpr
    void _drz_impl(std::complex<T>& complex_) noexcept
    {
      _stz_impl_IDEBUGGING("std::complex<%s>", _underlying_name<T>());

      _drz_impl(complex_.real);
      _drz_impl(complex_.imag);
    }

    template<typename T>
    constexpr
    void _srz_impl(const std::atomic<T>& atomic_) noexcept
    {
      _stz_impl_IDEBUGGING("std::atomic<%s>", _underlying_name<T>());

      _srz_impl(static_cast<const T&>(atomic_));
    }

    template<typename T>
    constexpr
    void _drz_impl(std::atomic<T>& atomic_) noexcept
    {
      _stz_impl_IDEBUGGING("std::atomic<%s>", _underlying_name<T>());

      T value = {};
      _drz_impl(value);
      atomic_ = value;
    }

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl(const std::basic_string<T>& string_) noexcept
    {
      _stz_impl_IDEBUGGING("std::basic_string<%s>", _underlying_name<T>());

      _size_t_srz_impl(string_.size());

      if _stz_impl_CONSTEXPR_CPP17 _stz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _srz_impl_many_fundamentals(string_[0], string_.size());
      }
      else
      {
        for (const auto character : string_)
        {
          _srz_impl(character);
        }
      }
    }

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::basic_string<T>& string_) noexcept
    {
      _stz_impl_IDEBUGGING("std::basic_string<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_drz_impl(size);

      string_.resize(size);

      if _stz_impl_CONSTEXPR_CPP17 _stz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _drz_impl_many_fundamentals(string_[0], size);
      }
      else
      {
        for (auto& character : string_)
        {
          _drz_impl(character);
        }
      }
    }

    template<typename T, size_t N1>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl(const std::array<T, N1>& array_) noexcept
    {
      _stz_impl_IDEBUGGING("std::array<%s, %zu>", _underlying_name<T>(), N1);

      if _stz_impl_CONSTEXPR_CPP17 _stz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _srz_impl_many_fundamentals(array_[0], N1);
      }
      else
      {
        for (const auto& value : array_)
        {
          _srz_impl(value);
        }
      }
    }

    template<typename T, size_t N1>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::array<T, N1>& array_) noexcept
    {
      _stz_impl_IDEBUGGING("std::array<%s, %zu>", _underlying_name<T>(), N1);

      if _stz_impl_CONSTEXPR_CPP17 _stz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _drz_impl_many_fundamentals(array_[0], N1);
      }
      else
      {
        for (auto& value : array_)
        {
          _drz_impl(value);
        }
      }
    }

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl(const std::vector<T>& vector_) noexcept
    {
      _stz_impl_IDEBUGGING("std::vector<%s>", _underlying_name<T>());

      _size_t_srz_impl(vector_.size());

      if _stz_impl_CONSTEXPR_CPP17 _stz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _srz_impl_many_fundamentals(vector_[0], vector_.size());
      }
      else
      {
        for (const auto& value : vector_)
        {
          _srz_impl(value);
        }
      }
    }

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::vector<T>& vector_) noexcept
    {
      _stz_impl_IDEBUGGING("std::vector<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_drz_impl(size);

      vector_.resize(size);

      if _stz_impl_CONSTEXPR_CPP17 _stz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _drz_impl_many_fundamentals(vector_[0], vector_.size());
      }
      else
      {
        for (auto& value : vector_)
        {
          _drz_impl(value);
        }
      }
    }

    void _srz_impl(const std::vector<bool>& vector_) noexcept
    {
      _stz_impl_IDEBUGGING("std::vector<bool>");

      _size_t_srz_impl(vector_.size());

      for (const bool value : vector_)
      {
        _srz_impl(value);
      }
    }

    void _drz_impl(std::vector<bool>& vector_) noexcept
    {
      _stz_impl_IDEBUGGING("std::vector<bool>");

      size_t size = {};
      _size_t_drz_impl(size);

      vector_.resize(size);

      bool value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _drz_impl(value);
        vector_.push_back(value);
      }
    }

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl(const std::valarray<T>& valarray_) noexcept
    {
      _stz_impl_IDEBUGGING("std::valarray<%s>", _underlying_name<T>());

      _size_t_srz_impl(valarray_.size());

      if _stz_impl_CONSTEXPR_CPP17 _stz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _srz_impl_many_fundamentals(valarray_[0], valarray_.size());
      }
      else
      {
        for (const auto& value : valarray_)
        {
          _srz_impl(value);
        }
      }
    }

    template<typename T>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::valarray<T>& valarray_) noexcept
    {
      _stz_impl_IDEBUGGING("std::valarray<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_drz_impl(size);

      valarray_.resize(size);

      if _stz_impl_CONSTEXPR_CPP17 _stz_impl_EXPECTED(std::is_fundamental<T>::value)
      {
        _drz_impl_many_fundamentals(valarray_[0], size);
      }
      else
      {
        for (auto& value : valarray_)
        {
          _drz_impl(value);
        }
      }
    }

    template<size_t N1>
    _stz_impl_CONSTEXPR_CPP14
    void _srz_impl(const std::bitset<N1>& bitset_) noexcept
    {
      _stz_impl_IDEBUGGING("std::bitset<%zu>", N1);

      _srz_impl(bitset_, 1);
    }

    template<size_t N1>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::bitset<N1>& bitset_) noexcept
    {
      _stz_impl_IDEBUGGING("std::bitset<%zu>", N1);

      _drz_impl(bitset_, 1);
    }

    template<typename T>
    constexpr
    void _srz_impl(const std::list<T>& list_) noexcept
    {
      _stz_impl_IDEBUGGING("std::list<%s>", _underlying_name<T>());

      _size_t_srz_impl(list_.size());

      for (const auto& value : list_)
      {
        _srz_impl(value);
      }
    }

    template<typename T>
    constexpr
    void _drz_impl(std::list<T>& list_) noexcept
    {
      _stz_impl_IDEBUGGING("std::list<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_drz_impl(size);

      list_.resize(size);
      for (auto& value : list_)
      {
        _drz_impl(value);
      }
    }

    template<typename T>
    constexpr
    void _srz_impl(const std::stack<T>& stack_) noexcept
    {
      _stz_impl_IDEBUGGING("std::stack<%s>", _underlying_name<T>());

      std::stack<T> temp = stack_;

      const auto size = stack_.size();
      _size_t_srz_impl(size);

      for (size_t k = size; k; --k)
      {
        _srz_impl(temp.top());
        temp.pop();
      }
    }

    template<typename T>
    constexpr
    void _drz_impl(std::stack<T>& stack_) noexcept
    {
      _stz_impl_IDEBUGGING("std::stack<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_drz_impl(size);

      std::stack<T> temp;

      T value = {};
      for (size_t k = size; k; --k)
      {
        _drz_impl(value);
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
    void _srz_impl(const std::forward_list<T>& forward_list_) noexcept
    {
      _stz_impl_IDEBUGGING("std::forward_list<%s>", _underlying_name<T>());

      size_t size = 0;
      for (auto iterator = forward_list_.begin(), end = forward_list_.end(); iterator != end; ++iterator)
      {
        ++size;
      }
      _size_t_srz_impl(size);

      for (const auto& value : forward_list_)
      {
        _srz_impl(value);
      }
    }

    template<typename T>
    constexpr
    void _drz_impl(std::forward_list<T>& forward_list_) noexcept
    {
      _stz_impl_IDEBUGGING("std::forward_list<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_drz_impl(size);

      forward_list_.resize(size);
      for (auto& value : forward_list_)
      {
        _drz_impl(value);
      }
    }

    template<typename T, typename S>
    constexpr
    void _srz_impl(const std::queue<T, S>& queue_) noexcept
    {
      _stz_impl_IDEBUGGING("std::queue<%s>", _underlying_name<T>());

      std::queue<T, S> temp = queue_;

      const auto size = queue_.size();
      _size_t_srz_impl(size);

      for (size_t k = size; k; --k)
      {
        _srz_impl(temp.front());
        temp.pop();
      }
    }

    template<typename T, typename S>
    constexpr
    void _drz_impl(std::queue<T, S>& queue_) noexcept
    {
      _stz_impl_IDEBUGGING("std::queue<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_drz_impl(size);

      queue_ = std::queue<T, S>();

      T value = {};
      for (size_t k = size; k; --k)
      {
        _drz_impl(value);
        queue_.push(std::move(value));
      }
    }

    template<typename T, class C, class F>
    constexpr
    void _srz_impl(const std::priority_queue<T, C, F>& priority_queue_) noexcept
    {
      _stz_impl_IDEBUGGING("std::priority_queue<%s>", _underlying_name<std::priority_queue<T, C, F>>());

      std::priority_queue<T, C, F> temp = priority_queue_;

      const auto size = priority_queue_.size();

      _size_t_srz_impl(size);

      for (size_t k = size; k; --k)
      {
        _srz_impl(temp.top());
        temp.pop();
      }
    }

    template<typename T, class C, class F>
    _stz_impl_CONSTEXPR_CPP14
    void _drz_impl(std::priority_queue<T, C, F>& priority_queue_) noexcept
    {
      _stz_impl_IDEBUGGING("std::priority_queue<%s>", _underlying_name<std::priority_queue<T, C, F>>());

      size_t size = 0;
      _size_t_drz_impl(size);

      priority_queue_ = std::priority_queue<T, C, F>();

      T value = {};
      for (size_t k = size; k; --k)
      {
        _drz_impl(value);
        priority_queue_.push(std::move(value));
      }
    }

    template<typename T, typename A>
    constexpr
    void _srz_impl(const std::deque<T, A>& deque_) noexcept
    {
      _stz_impl_IDEBUGGING("std::deque<%s>", _underlying_name<T>());

      _size_t_srz_impl(deque_.size());

      for (const auto& value : deque_)
      {
        _srz_impl(value);
      }
    }

    template<typename T, typename A>
    constexpr
    void _drz_impl(std::deque<T, A>& deque_) noexcept
    {
      _stz_impl_IDEBUGGING("std::deque<%s>", _underlying_name<T>());

      size_t size = {};
      _size_t_drz_impl(size);

      deque_.resize(size);
      for (auto& value : deque_)
      {
        _drz_impl(value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _srz_impl(const std::pair<T1, T2>& pair_) noexcept
    {
      _stz_impl_IDEBUGGING("std::pair<%s, %s>", _underlying_name<T1>(), _underlying_name<T2>());

      _srz_impl(pair_.first);
      _srz_impl(pair_.second);
    }

    template<typename T1, typename T2>
    constexpr
    void _drz_impl(std::pair<T1, T2>& pair_) noexcept
    {
      _stz_impl_IDEBUGGING("std::pair<%s, %s>", _underlying_name<T1>(), _underlying_name<T2>());

      _drz_impl(pair_.first);
      _drz_impl(pair_.second);
    }

    template<typename T1, typename T2>
    constexpr
    void _srz_impl(const std::unordered_map<T1, T2>& unordered_map_) noexcept
    {
      _stz_impl_IDEBUGGING("std::unordered_map");

      _size_t_srz_impl(unordered_map_.size());

      for (const auto& key_value : unordered_map_)
      {
        _srz_impl(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _drz_impl(std::unordered_map<T1, T2>& unordered_map_) noexcept
    {
      _stz_impl_IDEBUGGING("std::unordered_map");

      size_t size = {};
      _size_t_drz_impl(size);

      unordered_map_.clear();
      unordered_map_.reserve(size);

      std::pair<T1, T2> key_value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _drz_impl(key_value);
        unordered_map_.insert(std::move(key_value));
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _srz_impl(const std::unordered_multimap<T1, T2>& unordered_multimap_) noexcept
    {
      _stz_impl_IDEBUGGING("std::unordered_multimap");

      _size_t_srz_impl(unordered_multimap_.size());

      for (const auto& key_value : unordered_multimap_)
      {
        _srz_impl(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _drz_impl(std::unordered_multimap<T1, T2>& unordered_multimap_) noexcept
    {
      _stz_impl_IDEBUGGING("std::unordered_multimap");

      size_t size = {};
      _size_t_drz_impl(size);

      unordered_multimap_.clear();
      unordered_multimap_.reserve(size);

      std::pair<T1, T2> key_value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _drz_impl(key_value);
        unordered_multimap_.insert(std::move(key_value));
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _srz_impl(const std::map<T1, T2>& map_) noexcept
    {
      _stz_impl_IDEBUGGING("std::map");

      _size_t_srz_impl(map_.size());

      for (const auto& key_value : map_)
      {
        _srz_impl(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _drz_impl(std::map<T1, T2>& map_) noexcept
    {
      _stz_impl_IDEBUGGING("std::map");

      size_t size = {};
      _size_t_drz_impl(size);

      map_.clear();

      std::pair<T1, T2> key_value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _drz_impl(key_value);
        map_.insert(std::move(key_value));
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _srz_impl(const std::multimap<T1, T2>& multimap_) noexcept
    {
      _stz_impl_IDEBUGGING("std::multimap");

      _size_t_srz_impl(multimap_.size());

      for (const auto& key_value : multimap_)
      {
        _srz_impl(key_value);
      }
    }

    template<typename T1, typename T2>
    constexpr
    void _drz_impl(std::multimap<T1, T2>& multimap_) noexcept
    {
      _stz_impl_IDEBUGGING("std::multimap");

      size_t size = {};
      _size_t_drz_impl(size);

      multimap_.clear();

      std::pair<T1, T2> key_value = {};
      for (size_t k = 0; k < size; ++k)
      {
        _drz_impl(key_value);
        multimap_.insert(std::move(key_value));
      }
    }

    template<typename T>
    constexpr
    void _srz_impl(const std::unordered_set<T>& unordered_set_) noexcept
    {
      _stz_impl_IDEBUGGING("std::unordered_set");

      _size_t_srz_impl(unordered_set_.size());

      for (const auto& key : unordered_set_)
      {
        _srz_impl(key);
      }
    }

    template<typename T>
    constexpr
    void _drz_impl(std::unordered_set<T>& unordered_set_) noexcept
    {
      _stz_impl_IDEBUGGING("std::unordered_set");

      size_t size = {};
      _size_t_drz_impl(size);

      unordered_set_.clear();
      unordered_set_.reserve(size);

      T key = {};
      for (size_t k = 0; k < size; ++k)
      {
        _drz_impl(key);
        unordered_set_.insert(std::move(key));
      }
    }

    template<typename T>
    constexpr
    void _srz_impl(const std::unordered_multiset<T>& unordered_multiset_) noexcept
    {
      _stz_impl_IDEBUGGING("std::unordered_multiset");

      _size_t_srz_impl(unordered_multiset_.size());

      for (const auto& key : unordered_multiset_)
      {
        _srz_impl(key);
      }
    }

    template<typename T>
    constexpr
    void _drz_impl(std::unordered_multiset<T>& unordered_multiset_) noexcept
    {
      _stz_impl_IDEBUGGING("std::unordered_multiset");

      size_t size = {};
      _size_t_drz_impl(size);

      unordered_multiset_.clear();
      unordered_multiset_.reserve(size);

      T key = {};
      for (size_t k = 0; k < size; ++k)
      {
        _drz_impl(key);
        unordered_multiset_.insert(std::move(key));
      }
    }

    template<typename T>
    constexpr
    void _srz_impl(const std::set<T>& set_) noexcept
    {
      _stz_impl_IDEBUGGING("std::set");

      _size_t_srz_impl(set_.size());

      for (const auto& key : set_)
      {
        _srz_impl(key);
      }
    }

    template<typename T>
    constexpr
    void _drz_impl(std::set<T>& set_) noexcept
    {
      _stz_impl_IDEBUGGING("std::set");

      size_t size = {};
      _size_t_drz_impl(size);

      set_.clear();

      T key = {};
      for (size_t k = 0; k < size; ++k)
      {
        _drz_impl(key);
        set_.insert(std::move(key));
      }
    }

    template<typename T>
    constexpr
    void _srz_impl(const std::multiset<T>& multiset_) noexcept
    {
      _stz_impl_IDEBUGGING("std::multiset");

      _size_t_srz_impl(multiset_.size());

      for (const auto& key : multiset_)
      {
        _srz_impl(key);
      }
    }

    template<typename T>
    constexpr
    void _drz_impl(std::multiset<T>& multiset_) noexcept
    {
      _stz_impl_IDEBUGGING("std::multiset");

      size_t size = {};
      _size_t_drz_impl(size);

      multiset_.clear();

      T key = {};
      for (size_t k = 0; k < size; ++k)
      {
        _drz_impl(key);
        multiset_.insert(std::move(key));
      }
    }

    template<size_t N1, typename... T>
    struct _tuple_srz
    {
      static inline constexpr
      void _implementation(const std::tuple<T...>& tuple) noexcept;
    };

    template<typename... T>
    struct _tuple_srz<0, T...>
    {
      static inline constexpr
      void _implementation(const std::tuple<T...>&) noexcept {}
    };

    template<size_t N, typename... T>
    constexpr
    void _tuple_srz<N, T...>::_implementation(const std::tuple<T...>& tuple_) noexcept
    {
      _srz_impl(std::get<sizeof...(T) - N>(tuple_));
      _tuple_srz<N-1, T...>::_implementation(tuple_);
    }

    template<typename... T>
    constexpr
    void _srz_impl(const std::tuple<T...>& tuple_) noexcept
    {
      _stz_impl_IDEBUGGING("std::tuple");

      _tuple_srz<sizeof...(T), T...>::_implementation(tuple_);
    }

    template<size_t N1, typename... T>
    struct _tuple_drz
    {
      static inline constexpr
      void _implementation(std::tuple<T...>& tuple) noexcept;
    };

    template<typename... T>
    struct _tuple_drz<0, T...>
    {
      static inline constexpr
      void _implementation(std::tuple<T...>&) noexcept {}
    };

    template<size_t N, typename... T>
    constexpr
    void _tuple_drz<N, T...>::_implementation(std::tuple<T...>& tuple_) noexcept
    {
      _drz_impl(std::get<sizeof...(T) - N>(tuple_));
      _tuple_drz<N-1, T...>::_implementation(tuple_);
    }

    template<typename... T>
    constexpr
    void _drz_impl(std::tuple<T...>& tuple_) noexcept
    {
      _stz_impl_IDEBUGGING("std::tuple");

      _tuple_drz<sizeof...(T), T...>::_implementation(tuple_);
    }
  }
//*///------------------------------------------------------------------------------------------------------------------
  template<typename... T>
  _stz_impl_NODISCARD_REASON("serialize: ignoring the return value makes no sens.")
  auto serialize(const T&... things_) noexcept -> Bytes
  {
    _stz_impl_IDEBUGGING("serialization summary:");

    _seiriakos_impl::_buffer.clear();
    _seiriakos_impl::_buffer.reserve(_seiriakos_impl::_sizeof_many<T...>());

    _seiriakos_impl::_srz_dispatch(things_...);

    return _seiriakos_impl::_buffer;
  }
//*///------------------------------------------------------------------------------------------------------------------
  template<typename... T>
  void deserialize(const Byte data_[], const size_t size_, T&... things_) noexcept
  {
    _stz_impl_IDEBUGGING("deserialization summary:");

    _seiriakos_impl::_buffer.assign(data_, data_ + size_);
    _seiriakos_impl::_buffer_front = 0;

    _seiriakos_impl::_drz_dispatch(things_...);
  }
//*///------------------------------------------------------------------------------------------------------------------
  template<typename type>
  _stz_impl_NODISCARD_REASON("deserialize: ignoring the return value makes no sens.")
  type deserialize(const Byte data_[], const size_t size_) noexcept
  {
    type thing;
    
    deserialize(data_, size_, thing);

    return thing;
  }
//*///------------------------------------------------------------------------------------------------------------------
# undef  serialization_methods
    constexpr int serialization_methods() noexcept { return 0; }
# define serialization_methods()                                           \
    Bytes serialize() const noexcept                                       \
    {                                                                      \
      return stz::serialize(*this);                                        \
    }                                                                      \
    void deserialize(const stz::Byte data_[], const size_t size_) noexcept \
    {                                                                      \
      return stz::deserialize(data_, size_, *this);                        \
    }
//*///------------------------------------------------------------------------------------------------------------------
/*
# undef  serialization_sequence
    constexpr int serialization_sequence() noexcept { return 0; }
# define serialization_sequence(...)           \
      _seiriakos_impl::_backdoor friend;       \
    private:                                   \
      void _stz_impl_srz_seq() const noexcept  \
      {                                        \
        _stz_impl_MAYBE_UNUSED                 \
        stz::_seiriakos_impl::_srz serializer; \
        __VA_ARGS__                            \
      }                                        \
      void _stz_impl_drz_seq() noexcept        \
      {                                        \
        _stz_impl_MAYBE_UNUSED                 \
        stz::_seiriakos_impl::_drz serializer; \
        __VA_ARGS__                            \
      }
//*///------------------------------------------------------------------------------------------------------------------
/*
# undef  trivial_serialization
    constexpr int trivial_serialization() noexcept { return 0; }
# define trivial_serialization(...)                       \
      _seiriakos_impl::_backdoor friend;                  \
    private:                                              \
      void _stz_impl_srz_seq() const noexcept             \
      {                                                   \
        _stz_impl_assert_trivial(#__VA_ARGS__);           \
        stz::_seiriakos_impl::_srz_dispatch(__VA_ARGS__); \
      }                                                   \
      void _stz_impl_drz_seq() noexcept                   \
      {                                                   \
        stz::_seiriakos_impl::_drz_dispatch(__VA_ARGS__); \
      }
//*///------------------------------------------------------------------------------------------------------------------
# undef serialization_sequential
    constexpr int serialization_sequential() noexcept { return 0; }
# define serialization_sequential(...)                    \
      _seiriakos_impl::_backdoor friend;                  \
    private:                                              \
      void _stz_impl_srz_seq() const noexcept             \
      {                                                   \
        _stz_impl_assert_trivial(#__VA_ARGS__);           \
        stz::_seiriakos_impl::_srz_dispatch(__VA_ARGS__); \
      }                                                   \
      void _stz_impl_drz_seq() noexcept                   \
      {                                                   \
        stz::_seiriakos_impl::_drz_dispatch(__VA_ARGS__); \
      }
//*///------------------------------------------------------------------------------------------------------------------
# undef  serialization_procedural
    constexpr int serialization_procedural() noexcept { return 0; }
# define serialization_procedural(...)         \
      _seiriakos_impl::_backdoor friend;       \
    private:                                   \
      void _stz_impl_srz_seq() const noexcept  \
      {                                        \
        _stz_impl_MAYBE_UNUSED                 \
        stz::_seiriakos_impl::_srz serializer; \
        __VA_ARGS__                            \
      }                                        \
      void _stz_impl_drz_seq() noexcept        \
      {                                        \
        _stz_impl_MAYBE_UNUSED                 \
        stz::_seiriakos_impl::_drz serializer; \
        __VA_ARGS__                            \
      }
//*///------------------------------------------------------------------------------------------------------------------/*
/*
# define STZ_MAKE_SERIALIZATION(TYPE, ...)                                            \
    namespace stz                                                                     \
    {                                                                                 \
      static_assert(_seiriakos_impl::_force_global_scope::value, "");                 \
      inline namespace seiriakos                                                      \
      {                                                                               \
        auto serialize(const TYPE& instance) noexcept -> Bytes                        \
        {                                                                             \
          _stz_impl_IDEBUGGING("serialization summary:");                             \
          _impl::_buffer.clear();                                                     \
          _impl::_buffer.reserve(sizeof(TYPE));                                       \
          {                                                                           \
          _stz_impl_GCC_CLANG_IGNORE("-Wshadow",                                      \
          _stz_impl_IDEBUGGING("%s", stz::_seiriakos_impl::_underlying_name<TYPE>()); \
          )                                                                           \
          _stz_impl_MAYBE_UNUSED                                                      \
          constexpr stz::_seiriakos_impl::_srz serialize;                             \
          __VA_ARGS__                                                                 \
          }                                                                           \
          return _impl::_buffer;                                                      \
        }                                                                             \
        template<>                                                                    \
        TYPE deserialize<TYPE>(const Byte data[], size_t size) noexcept            \
        {                                                                             \
          _stz_impl_MAYBE_UNUSED                                                      \
          constexpr stz::_seiriakos_impl::_drz serialize;                             \
          TYPE instance;                                                              \
          __VA_ARGS__                                                                 \
          return instance;                                                            \
        }                                                                             \
      }                                                                               \
    }
//*///------------------------------------------------------------------------------------------------------------------
  template<unsigned size, typename type>
  auto bitfield(const type bitfield_data_) -> Bitfield<size, type>
  {
    static _stz_impl_THREADLOCAL Bitfield<size, type> bitfield;

    bitfield.proxy = bitfield_data_;

    return bitfield;
  }
//*///------------------------------------------------------------------------------------------------------------------
  auto hex_string(const Byte data[], const size_t size) -> const char*
  {
    static _stz_impl_THREADLOCAL std::vector<char> buffer;

    if _stz_impl_ABNORMAL(data == nullptr)
    {
      _stz_impl_DEBUGGING("'data' is nullptr.");
      return nullptr;
    }

    if ((3*size + 1) > buffer.capacity())
    {
      buffer.reserve(3*size + 1);
    }

    buffer.clear();

    constexpr char character[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    for (size_t k = 0; k < size; ++k)
    {
      // const char nybl_hi = data[k] >> 4;
      // const char nybl_lo = data[k] & 0xF;

      // buffer.push_back((nybl_hi <= 0x9) ? (nybl_hi + '0') : (nybl_hi + 'A' - 10));
      // buffer.push_back((nybl_lo <= 0x9) ? (nybl_lo + '0') : (nybl_lo + 'A' - 10));
      buffer.push_back(character[data[k] >>  4]);
      buffer.push_back(character[data[k] & 0xF]);

      buffer.push_back(' ');
    }

    buffer[buffer.size() - 1] = '\0';

    return buffer.data();
  }
//*///------------------------------------------------------------------------------------------------------------------
} /* namespace seiriakos */
} /* namespace stz       */
//*///------------------------------------------------------------------------------------------------------------------
#undef _stz_impl_PRAGMA
#undef _stz_impl_CLANG_IGNORE
#undef _stz_impl_GCC_IGNORE
#undef _stz_impl_GCC_CLANG_IGNORE
#undef _stz_impl_THREADLOCAL
#undef _stz_impl_ATOMIC
#undef _stz_impl_DECLARE_MUTEX
#undef _stz_impl_DECLARE_LOCK
#undef _stz_impl_LIKELY
#undef _stz_impl_UNLIKELY
#undef _stz_impl_EXPECTED
#undef _stz_impl_ABNORMAL
#undef _stz_impl_NODISCARD
#undef _stz_impl_NODISCARD_REASON
#undef _stz_impl_IDEBUGGING
#undef _stz_impl_DEBUGGING
#undef _stz_impl_WARNING
#undef _stz_impl_CONSTEXPR_CPP14
#undef _stz_impl_CONSTEXPR_CPP17
//*///------------------------------------------------------------------------------------------------------------------
#else
#error "stz: Support for ISO C++11 is required."
#endif
#endif