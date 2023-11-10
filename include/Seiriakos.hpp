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
#ifndef SEIRIAKOS_H
#define SEIRIAKOS_H
//---necessary standard libraries-------------------------------------------------------------------
#include <cstddef>     // for size_t
#include <cstdint>     // for uint8_t, UINT8_MAX, UINT16_MAX, UINT32_MAX
#include <vector>      // for std::vector
#include <type_traits> // for std::is_base_of
#include <iostream>    // for std::cout, std::cerr
#include <iomanip>     // for std::setw, std::setfill, std::hex
#include <ios>         // for std::uppercase 
#ifdef SEIRIAKOS_LOGGING
# include <typeinfo>   // for typeid
# include <cxxabi.h>   // for abi::__cxa_demangle
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
    const unsigned long MAJOR = 000;
    const unsigned long MINOR = 001;
    const unsigned long PATCH = 000;
    constexpr unsigned long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }

  namespace Global
  {
    std::ostream out{std::cout.rdbuf()}; // output ostream
    std::ostream err{std::cerr.rdbuf()}; // error ostream
    std::ostream log{std::clog.rdbuf()}; // logging ostream
  }
//----------------------------------------------------------------------------------------------------------------------
  inline namespace Frontend
  {
    // abstract class to access serialization capabilities
    class Serializable;

    // macro to implement quickly serialization/deserialization from a list of variables
#   define SEIRIAKOS_SEQUENCE(...)
    
    // print all bytes of an array of bytes
    inline void print_bytes(const uint8_t data[], const size_t size, const bool print_header = true);
  }
//----------------------------------------------------------------------------------------------------------------------
  namespace Backend
  {
    void error(const char* message) noexcept;

# if defined(__STDCPP_THREADS__) and (__STDCPP_THREADS__ == 1)
    static thread_local std::vector<uint8_t> buffer;
    static thread_local size_t front_of_buffer;
    static thread_local bool error_flag;
# else
    static std::vector<uint8_t> buffer;
    static size_t front_of_buffer;
    static bool error_flag;
#   endif

# if defined(SEIRIAKOS_LOGGING)
    class IndentLog
    {
      public:
        inline IndentLog(const std::string& text) noexcept
        {
          std::clog << "log: " << indentation << text << std::endl;
          indentation += "  ";
        }

        inline ~IndentLog() noexcept
        {
          indentation.resize(indentation.length() - 2);
        }
      private:
        static std::string indentation;
    };
    std::string IndentLog::indentation;

#   define SEIRIAKOS_LOG(text) Backend::IndentLog inde_nt_l_og_{text} // mangled name to avoid name collision
# else
#   define SEIRIAKOS_LOG(text) /* to enable logging #define SEIRIAKOS_LOGGING */
# endif

    template<typename T>
    using disable_if_Serializable = typename std::enable_if<!std::is_base_of<Serializable, T>::value>::type;

    template<typename T>
    inline std::string underlying_name();

    inline void serialization_implementation(const Serializable& data);
    inline void deserialization_implementation(Serializable& data);

    template<typename T, typename = disable_if_Serializable<T>>
    inline void serialization_implementation(const T& data);
    template<typename T, typename = disable_if_Serializable<T>>
    inline void deserialization_implementation(T& data);

    inline void size_t_serialization_implementation(const size_t& size);
    inline void size_t_deserialization_implementation(size_t& size);

    template<typename T>
    inline void serialization_implementation(const std::complex<T>& complex);
    template<typename T>
    inline void deserialization_implementation(std::complex<T>& complex);

    template<typename T>
    inline void serialization_implementation(const std::basic_string<T>& string);
    template<typename T>
    inline void deserialization_implementation(std::basic_string<T>& string);

    template<typename T>
    inline void serialization_implementation(const std::vector<T>& vector);
    template<typename T>
    inline void deserialization_implementation(std::vector<T>& vector);

    template<typename T>
    inline void serialization_implementation(const std::list<T>& list);
    template<typename T>
    inline void deserialization_implementation(std::list<T>& list);

    template<typename T>
    inline void serialization_implementation(const std::deque<T>& deque);
    template<typename T>
    inline void deserialization_implementation(std::deque<T>& deque);

    template<typename T1, typename T2>
    inline void serialization_implementation(const std::pair<T1, T2>& pair);
    template<typename T1, typename T2>
    inline void deserialization_implementation(std::pair<T1, T2>& pair);

    template<typename T1, typename T2>
    inline void serialization_implementation(const std::unordered_map<T1, T2>& unordered_map);
    template<typename T1, typename T2>
    inline void deserialization_implementation(std::unordered_map<T1, T2>& unordered_map);

    template<typename T1, typename T2>
    inline void serialization_implementation(const std::unordered_multimap<T1, T2>& unordered_multimap);
    template<typename T1, typename T2>
    inline void deserialization_implementation(std::unordered_multimap<T1, T2>& unordered_multimap);

    template<typename T1, typename T2>
    inline void serialization_implementation(const std::map<T1, T2>& map);
    template<typename T1, typename T2>
    inline void deserialization_implementation(std::map<T1, T2>& map);

    template<typename T1, typename T2>
    inline void serialization_implementation(const std::multimap<T1, T2>& multimap);
    template<typename T1, typename T2>
    inline void deserialization_implementation(std::multimap<T1, T2>& multimap);

    template<typename T>
    inline void serialization_implementation(const std::unordered_set<T>& unordered_set);
    template<typename T>
    inline void deserialization_implementation(std::unordered_set<T>& unordered_set);

    template<typename T>
    inline void serialization_implementation(const std::unordered_multiset<T>& unordered_multiset);
    template<typename T>
    inline void deserialization_implementation(std::unordered_multiset<T>& unordered_multiset);

    template<typename T>
    inline void serialization_implementation(const std::set<T>& set);
    template<typename T>
    inline void deserialization_implementation(std::set<T>& set);

    template<typename T>
    inline void serialization_implementation(const std::multiset<T>& multiset);
    template<typename T>
    inline void deserialization_implementation(std::multiset<T>& multiset);

    template<typename... T>
    inline void serialization_implementation(const std::tuple<T...>& tuple);
    template<typename... T>
    inline void deserialization_implementation(std::multiset<T...>& tuple);
  }
//----------------------------------------------------------------------------------------------------------------------
  inline namespace Frontend
  {
    class Serializable
    {
      public:
        // serialize *this
        inline std::vector<uint8_t> serialize() const noexcept;
        // deserialize data into *this
        inline bool deserialize(const uint8_t data[], const size_t size) noexcept;
      protected:
        // serialization/deserialization sequence (provided by the inheriting class)
        virtual void serialization_sequence() const noexcept = 0;
        virtual void deserialization_sequence() noexcept = 0;
        // recursive calls to appropriate serialization_implementation overloads
        template<typename T, typename... Tn>
        inline void serialization(const T& data, const Tn&... data_n) const noexcept;
        inline void serialization() const noexcept {};
        // recursive calls to appropriate deserialization_implementation overloads
        template<typename T, typename... Tn>
        inline void deserialization(T& data, Tn&... data_n) noexcept;
        inline void deserialization() noexcept {};
      // to allow access to serialization_sequence/deserialization_sequence
      friend void Backend::serialization_implementation(const Serializable& data);
      friend void Backend::deserialization_implementation(Serializable& data);
    };
  }
//----------------------------------------------------------------------------------------------------------------------
  inline namespace Frontend
  {
    std::vector<uint8_t> Serializable::serialize() const noexcept
    {
      Backend::buffer.clear(); // initialize buffer

      // call the sequence provided by the inheriting class
      serialization_sequence();

      return Backend::buffer; // return serialized data
    }

    bool Serializable::deserialize(const uint8_t data[], const size_t size) noexcept
    {
      Backend::buffer.assign(data, data + size); // initialize buffer with data
      Backend::front_of_buffer = 0;              // initialize front of buffer
      Backend::error_flag      = false;          // initialize error flag

      // call the sequence provided by the inheriting class
      deserialization_sequence();

      if (Backend::front_of_buffer != Backend::buffer.size())
      {
        Backend::error("buffer is not empty, serialization/deserialization sequence mismatch");
      }

      return Backend::error_flag;
    }

    template<typename T, typename... Tn>
    void Serializable::serialization(const T& data, const Tn&... data_n) const noexcept
    {
      Backend::serialization_implementation(data);
      serialization(data_n...);
    }

    template<typename T, typename... Tn>
    void Serializable::deserialization(T& data, Tn&... data_n) noexcept
    {
      Backend::deserialization_implementation(data);
      deserialization(data_n...);
    }

    #undef  SEIRIAKOS_SEQUENCE
    #define SEIRIAKOS_SEQUENCE(...)                           \
      private:                                                \
        void serialization_sequence() const noexcept override \
        {                                                     \
          serialization(__VA_ARGS__);                         \
        }                                                     \
        void deserialization_sequence() noexcept override     \
        {                                                     \
          deserialization(__VA_ARGS__);                       \
        }
  }
//----------------------------------------------------------------------------------------------------------------------
  namespace Backend
  {
    void error(const char* message) noexcept
    {
      error_flag = true;
      Global::err << "error: Seiriakos: " << message << std::endl;
    }

    void serialization_implementation(const Serializable& data)
    {
      SEIRIAKOS_LOG("Serializable");

      // serialize data via its specialized implementation
      data.serialization_sequence();
    }

    void deserialization_implementation(Serializable& data)
    {
      SEIRIAKOS_LOG("Serializable");

      // serialize data via its specialized implementation
      data.deserialization_sequence();
    }

# if defined(SEIRIAKOS_LOGGING)
    template<typename T>
    std::string underlying_name()
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
        int status;
        char* demangled = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
        std::string demangled_name = demangled;
        std::free(demangled);
        return demangled_name;
      }
    }
# endif

    template<typename T, typename>
    void serialization_implementation(const T& data)
    {
      SEIRIAKOS_LOG(underlying_name<T>());

      // add data's bytes one by one to the buffer
      for (size_t k = 0; k < sizeof(T); ++k)
      {
        buffer.push_back(reinterpret_cast<const uint8_t*>(&data)[k]);
      }
    }

    template<typename T, typename>
    void deserialization_implementation(T& data)
    {
      SEIRIAKOS_LOG(underlying_name<T>());

      if (front_of_buffer >= buffer.size())
      {
        error("could not deserialize data, buffer is empty");
        return;
      }

      if ((buffer.size() - front_of_buffer) < sizeof(T))
      {
        error("could not deserialize data, not enough bytes in buffer");
        return;
      }

      // set data's bytes one by one from the front of the buffer
      for (size_t k = 0; k < sizeof(T); ++k)
      {
        reinterpret_cast<uint8_t*>(&data)[k] = buffer[front_of_buffer++];
      }
    }

    void size_t_serialization_implementation(const size_t& size)
    {
      SEIRIAKOS_LOG("size_t");

      // compute minimum amount of bytes needed to serialize
      uint8_t bytes_used = 1;
      for (size_t k = size; k >>= 8; ++bytes_used)
      {}

      buffer.push_back(bytes_used);

      for (size_t k = 0; bytes_used--; k += 8)
      {
        buffer.push_back(static_cast<uint8_t>((size >> k) & 0xFF));
      }
    }

    void size_t_deserialization_implementation(size_t& size)
    {
      SEIRIAKOS_LOG("size_t");

      if (front_of_buffer >= buffer.size())
      {
        error("could not deserialize data, buffer is empty");
        return;
      }

      uint8_t bytes_used = buffer[front_of_buffer++];

      if ((buffer.size() - front_of_buffer) < bytes_used)
      {
        error("could not deserialize data, not enough bytes in buffer");
        return;
      }

      size = 0;
      for (size_t k = 0; bytes_used--; k += 8)
      {
        size |= (buffer[front_of_buffer++] << k);
      }
    }

    template<typename T>
    inline void serialization_implementation(const std::complex<T>& complex)
    {
      SEIRIAKOS_LOG("std::complex");

      serialization_implementation(complex.real);
      serialization_implementation(complex.imag);
    }

    template<typename T>
    inline void deserialization_implementation(std::complex<T>& complex)
    {
      SEIRIAKOS_LOG("std::complex");
      
      deserialization_implementation(complex.real);
      deserialization_implementation(complex.imag);
    }

    template<typename T>
    void serialization_implementation(const std::basic_string<T>& string)
    {
      SEIRIAKOS_LOG("std::string");

      size_t_serialization_implementation(string.size());

      for (T character : string)
      {
        serialization_implementation(character);
      }
    }

    template<typename T>
    void deserialization_implementation(std::basic_string<T>& string)
    {
      SEIRIAKOS_LOG("std::string");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      string.clear();

      T character = '\n';
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(character);
        string += character;
      }
    }

    template<typename T>
    void serialization_implementation(const std::vector<T>& vector)
    {
      SEIRIAKOS_LOG("std::vector");

      size_t_serialization_implementation(vector.size());

      for (const T& value : vector)
      {
        serialization_implementation(value);
      }
    }

    template<typename T>
    void deserialization_implementation(std::vector<T>& vector)
    {
      SEIRIAKOS_LOG("std::vector");

      vector.clear();

      size_t size = 0;
      size_t_deserialization_implementation(size);

      vector.reserve(size);

      T value;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(value);
        vector.push_back(value);
      }
    }

    template<typename T>
    void serialization_implementation(const std::list<T>& list)
    {
      SEIRIAKOS_LOG("std::list");

      size_t_serialization_implementation(list.size());

      for (const T& value : list)
      {
        serialization_implementation(value);
      }
    }

    template<typename T>
    void deserialization_implementation(std::list<T>& list)
    {
      SEIRIAKOS_LOG("std::list");

      list.clear();

      size_t size = 0;
      size_t_deserialization_implementation(size);

      T value;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(value);
        list.push_back(value);
      }
    }

    template<typename T>
    void serialization_implementation(const std::deque<T>& deque)
    {
      SEIRIAKOS_LOG("std::deque");

      size_t_serialization_implementation(deque.size());

      for (const T& value : deque)
      {
        serialization_implementation(value);
      }
    }

    template<typename T>
    void deserialization_implementation(std::deque<T>& deque)
    {
      SEIRIAKOS_LOG("std::deque");

      deque.clear();

      size_t size = 0;
      size_t_deserialization_implementation(size);

      T value;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(value);
        deque.push_back(value);
      }
    }

    template<typename T1, typename T2>
    void serialization_implementation(const std::pair<T1, T2>& pair)
    {
      SEIRIAKOS_LOG("std::pair");

      serialization_implementation(pair.first);
      serialization_implementation(pair.second);
    }

    template<typename T1, typename T2>
    void deserialization_implementation(std::pair<T1, T2>& pair)
    {
      SEIRIAKOS_LOG("std::pair");

      deserialization_implementation(pair.first);
      deserialization_implementation(pair.second);
    }

    template<typename T1, typename T2>
    void serialization_implementation(const std::unordered_map<T1, T2>& unordered_map)
    {
      SEIRIAKOS_LOG("std::unordered_map");

      size_t_serialization_implementation(unordered_map.size());

      for (const std::pair<const T1, T2>& key_value : unordered_map)
      {
        serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    void deserialization_implementation(std::unordered_map<T1, T2>& unordered_map)
    {
      SEIRIAKOS_LOG("std::unordered_map");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      unordered_map.clear();

      std::pair<T1, T2> key_value;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(key_value);
        unordered_map.insert(key_value);
      }
    }

    template<typename T1, typename T2>
    void serialization_implementation(const std::unordered_multimap<T1, T2>& unordered_multimap)
    {
      SEIRIAKOS_LOG("std::unordered_multimap");
      
      size_t_serialization_implementation(unordered_multimap.size());

      for (const std::pair<const T1, T2>& key_value : unordered_multimap)
      {
        serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    void deserialization_implementation(std::unordered_multimap<T1, T2>& unordered_multimap)
    {
      SEIRIAKOS_LOG("std::unordered_multimap");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      unordered_multimap.clear();

      std::pair<T1, T2> key_value;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(key_value);
        unordered_multimap.insert(key_value);
      }
    }

    template<typename T1, typename T2>
    void serialization_implementation(const std::map<T1, T2>& map)
    {
      SEIRIAKOS_LOG("std::map");
      
      size_t_serialization_implementation(map.size());

      for (const std::pair<const T1, T2>& key_value : map)
      {
        serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    void deserialization_implementation(std::map<T1, T2>& map)
    {
      SEIRIAKOS_LOG("std::map");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      map.clear();

      std::pair<T1, T2> key_value;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(key_value);
        map.insert(key_value);
      }
    }

    template<typename T1, typename T2>
    void serialization_implementation(const std::multimap<T1, T2>& multimap)
    {
      SEIRIAKOS_LOG("std::multimap");
      
      size_t_serialization_implementation(multimap.size());

      for (const std::pair<const T1, T2>& key_value : multimap)
      {
        serialization_implementation(key_value);
      }
    }

    template<typename T1, typename T2>
    void deserialization_implementation(std::multimap<T1, T2>& multimap)
    {
      SEIRIAKOS_LOG("std::multimap");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      multimap.clear();

      std::pair<T1, T2> key_value;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(key_value);
        multimap.insert(key_value);
      }
    }

    template<typename T>
    void serialization_implementation(const std::unordered_set<T>& unordered_set)
    {
      SEIRIAKOS_LOG("std::unordered_set");

      size_t_serialization_implementation(unordered_set.size());

      for (const T& key : unordered_set)
      {
        serialization_implementation(key);
      }
    }

    template<typename T>
    void deserialization_implementation(std::unordered_set<T>& unordered_set)
    {
      SEIRIAKOS_LOG("std::unordered_set");

      size_t size = 0;
      size_t_deserialization_implementation(size);

      unordered_set.clear();

      T key;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(key);
        unordered_set.insert(key);
      }
    }

    template<typename T>
    void serialization_implementation(const std::unordered_multiset<T>& unordered_multiset)
    {
      SEIRIAKOS_LOG("std::unordered_multiset");
      
      size_t_serialization_implementation(unordered_multiset.size());

      for (const T& key : unordered_multiset)
      {
        serialization_implementation(key);
      }
    }

    template<typename T>
    void deserialization_implementation(std::unordered_multiset<T>& unordered_multiset)
    {
      SEIRIAKOS_LOG("std::unordered_multiset");
      
      size_t size = 0;
      size_t_deserialization_implementation(size);

      unordered_multiset.clear();

      T key;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(key);
        unordered_multiset.insert(key);
      }
    }

    template<typename T>
    void serialization_implementation(const std::set<T>& set)
    {
      SEIRIAKOS_LOG("std::set");
      
      size_t_serialization_implementation(set.size());

      for (const T& key : set)
      {
        serialization_implementation(key);
      }
    }

    template<typename T>
    void deserialization_implementation(std::set<T>& set)
    {
      SEIRIAKOS_LOG("std::set");
      
      size_t size = 0;
      size_t_deserialization_implementation(size);

      set.clear();

      T key;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(key);
        set.insert(key);
      }
    }

    template<typename T>
    void serialization_implementation(const std::multiset<T>& multiset)
    {
      SEIRIAKOS_LOG("std::multiset");
      
      size_t_serialization_implementation(multiset.size());

      for (const T& key : multiset)
      {
        serialization_implementation(key);
      }
    }

    template<typename T>
    void deserialization_implementation(std::multiset<T>& multiset)
    {
      SEIRIAKOS_LOG("std::multiset");
      
      size_t size = 0;
      size_t_deserialization_implementation(size);

      multiset.clear();

      T key;
      for (size_t k = 0; k < size; ++k)
      {
        deserialization_implementation(key);
        multiset.insert(key);
      }
    }

    template<typename... T>
    void serialization_implementation(const std::tuple<T...>&)
    {
      error("tuples are not implemented yet");
    }

    template<typename... T>
    void deserialization_implementation(std::tuple<T...>&)
    {
      error("tuples are not implemented yet");
    }
  }
//----------------------------------------------------------------------------------------------------------------------
  inline namespace Frontend
  {
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
  }

  #undef SEIRIAKOS_LOG
//----------------------------------------------------------------------------------------------------------------------
}
#endif