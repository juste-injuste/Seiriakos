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
deserialize objects. do #define SEIRIAKOS_THREADSAFE before including to enable thread-safety.

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
//-------------------
#include <vector>
#include <list>
#include <deque>
#include <string>
#include <utility>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <set>
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
  }
//--------------------------------------------------------------------------------------------------
  inline namespace Frontend
  {
    // abstract class to access serialization capabilities
    class Serializable;

    // macro to implement quickly serialization/deserialization from a list of variables
    #define SEIRIAKOS_SEQUENCE(...)

    // print all bytes of an array of bytes
    void print_bytes(const uint8_t data[], const size_t size, const bool print_header = true);
  }
//--------------------------------------------------------------------------------------------------
  namespace Backend
  {
    #ifdef SEIRIAKOS_NOT_THREADSAFE
      static std::vector<uint8_t> buffer;
      static size_t front_of_buffer;
    #else
      static thread_local std::vector<uint8_t> buffer;
      static thread_local size_t front_of_buffer;
    #endif

    template<class M>
    struct is_map_like : std::false_type {};

    template<class K, class T>
    struct is_map_like<std::map<K, T>> : std::true_type {};

    template<class K, class T>
    struct is_map_like<std::multimap<K, T>> : std::true_type {};

    template<class K, class T>
    struct is_map_like<std::unordered_map<K, T>> : std::true_type {};

    template<class K, class T>
    struct is_map_like<std::unordered_multimap<K, T>> : std::true_type {};

    template<class S>
    struct is_set_like : std::false_type {};

    template<class K>
    struct is_set_like<std::set<K>> : std::true_type {};

    template<class K>
    struct is_set_like<std::multiset<K>> : std::true_type {};

    template<class K>
    struct is_set_like<std::unordered_set<K>> : std::true_type {};

    template<class K>
    struct is_set_like<std::unordered_multiset<K>> : std::true_type {};

    template<typename T>
    using is_Serializable = std::is_base_of<Serializable, T>;

    template<class M>
    using enable_if_maplike = typename std::enable_if<is_map_like<M>::value>::type;

    template<class S>
    using enable_if_setlike = typename std::enable_if<is_set_like<S>::value>::type;

    template <typename T>
    struct is_generic : std::conditional<
      is_Serializable<T>::value ||
      is_map_like<T>::value ||
      is_set_like<T>::value,
      std::false_type, 
      std::true_type
    >::type {};

    // template<typename T>
    // using enable_if_generic = typename std::enable_if<is_generic<T>::value>::type;

    void data_serialization_implementation(const Serializable& data);
    void data_deserialization_implementation(Serializable& data);

    template<typename T>
    void data_serialization_implementation(const T& data, is_generic<T>* = nullptr);
    template<typename T>
    void data_deserialization_implementation(T& data, is_generic<T>* = nullptr);

    void size_serialization_implementation(const size_t& size);
    void size_deserialization_implementation(size_t& size);

    template<typename T>
    void data_serialization_implementation(const std::basic_string<T>& string);
    template<typename T>
    void data_deserialization_implementation(std::basic_string<T>& string);

    template<typename T>
    void data_serialization_implementation(const std::vector<T>& data);
    template<typename T>
    void data_deserialization_implementation(std::vector<T>& data);

    template<typename T>
    void data_serialization_implementation(const std::list<T>& data);
    template<typename T>
    void data_deserialization_implementation(std::list<T>& data);

    template<typename T>
    void data_serialization_implementation(const std::deque<T>& data);
    template<typename T>
    void data_deserialization_implementation(std::deque<T>& data);

    template<typename T1, typename T2>
    void data_serialization_implementation(const std::pair<T1, T2>& data);
    template<typename T1, typename T2>
    void data_deserialization_implementation(std::pair<T1, T2>& data);

    template<class M>
    void data_serialization_implementation(M& data, enable_if_maplike<M>* = nullptr);
    template<class M>
    void data_deserialization_implementation(M& data, enable_if_maplike<M>* = nullptr);

    template<class S>
    void data_serialization_implementation(const S& data, enable_if_setlike<S>* = nullptr);
    template<class S>
    void data_deserialization_implementation(S& data, enable_if_setlike<S>* = nullptr);
  }
//--------------------------------------------------------------------------------------------------
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

        // recursive calls to appropriate data_serialization/data_deserialization overloads
        template<typename T, typename... T_n>
        inline void data_serialization(const T& data, const T_n&... data_n) const noexcept;
        inline void data_serialization() const noexcept {};
        template<typename T, typename... T_n>
        inline void data_deserialization(T& data, T_n&... data_n) noexcept;
        inline void data_deserialization() noexcept {};

        friend void Backend::data_serialization_implementation(const Serializable& data);
        friend void Backend::data_deserialization_implementation(Serializable& data);
      private:
        // issue error message
        inline void error(const char* message) const noexcept;
        #ifdef SEIRIAKOS_NOT_THREADSAFE
          static bool error_flag;
        #else
          static thread_local bool error_flag;
        #endif
    };

    // Serializable static member initializations
    #ifdef SEIRIAKOS_NOT_THREADSAFE
      bool Serializable::error_flag;
    #else
      thread_local bool Serializable::error_flag;
    #endif
  }
//--------------------------------------------------------------------------------------------------
  inline namespace Frontend
  {
    std::vector<uint8_t> Serializable::serialize() const noexcept
    {
      Backend::buffer.clear(); // initialize buffer

      // call the implementation provided by the inheriting class
      serialization_sequence();

      return Backend::buffer; // return serialized data
    }

    bool Serializable::deserialize(const uint8_t data[], const size_t size) noexcept
    {
      Backend::buffer.assign(data, data + size); // initialize buffer with data
      Backend::front_of_buffer = 0;              // initialize front of buffer
      error_flag      = false;                   // initialize error flag

      // call the implementation provided by the inheriting class
      deserialization_sequence();
      
      if (Backend::front_of_buffer != Backend::buffer.size())
      {
        error("buffer is not empty, serialization/deserialization sequence mismatch");
      }

      return error_flag;
    }

    template<typename T, typename... T_n>
    void Serializable::data_serialization(const T& data, const T_n&... data_n) const noexcept
    {
      Backend::data_serialization_implementation(data);
      data_serialization(data_n...);
    }

    template<typename T, typename... T_n>
    void Serializable::data_deserialization(T& data, T_n&... data_n) noexcept
    {
      Backend::data_deserialization_implementation(data);
      data_deserialization(data_n...);
    }

    void Serializable::error(const char* message) const noexcept
    {
      error_flag = true;
      Global::err << "error: Serializable: " << message << std::endl;
    }

    #undef  SEIRIAKOS_SEQUENCE
    #define SEIRIAKOS_SEQUENCE(...)                           \
      private:                                                \
        void serialization_sequence() const noexcept override \
        {                                                     \
          data_serialization(__VA_ARGS__);                    \
        }                                                     \
        void deserialization_sequence() noexcept override     \
        {                                                     \
          data_deserialization(__VA_ARGS__);                  \
        }
  }
//--------------------------------------------------------------------------------------------------
  namespace Backend
  {
    void data_serialization_implementation(const Serializable& data)
    {
      // serialize data via its specialized implementation
      data.serialization_sequence();
    }

    void data_deserialization_implementation(Serializable& data)
    {
      // serialize data via its specialized implementation
      data.deserialization_sequence();
    }
//--------------------------------------------------------------------------------------------------
    template<typename T>
    void data_serialization_implementation(const T& data, is_generic<T>*)
    {
      // std::cout << "GENERIC SERIALIZATION\n";
      // add data's bytes one by one to the buffer
      for (size_t k = 0; k < sizeof(T); ++k)
      {
        buffer.push_back(reinterpret_cast<const uint8_t*>(&data)[k]);
      } 
    }

    template<typename T>
    void data_deserialization_implementation(T& data, is_generic<T>*)
    {
      // std::cout << "GENERIC DESERIALIZATION\n";
      if (front_of_buffer >= buffer.size())
      {
        // error("could not deserialize data, buffer is empty");
        return;
      }

      if ((buffer.size() - front_of_buffer) < sizeof(T))
      {
        // error("could not deserialize data, not enough bytes in buffer");
        return;
      }

      // set data's bytes one by one from the front of the buffer
      for (size_t k = 0; k < sizeof(T); ++k)
      {
        reinterpret_cast<uint8_t*>(&data)[k] = buffer[front_of_buffer++];
      }
    }
//--------------------------------------------------------------------------------------------------
    void size_serialization_implementation(const size_t& size)
    {
      uint8_t bytes_used = 8 - (size <= UINT8_MAX) - 2*(size <= UINT16_MAX) - 4*(size <= UINT32_MAX);

      buffer.push_back(bytes_used);

      for (size_t k = 0; k < bytes_used; ++k)
      {
        buffer.push_back(reinterpret_cast<const uint8_t*>(&size)[k]);
      } 
    }

    void size_deserialization_implementation(size_t& size)
    {
      // std::cout << "SIZE DESERIALIZATION\n";
      const uint8_t bytes_used = buffer[front_of_buffer++];

      size = 0;

      for (size_t k = 0; k < bytes_used; ++k)
      {
        reinterpret_cast<uint8_t*>(&size)[k] = buffer[front_of_buffer++];
      }
    }
//--------------------------------------------------------------------------------------------------
    template<typename T>
    void data_serialization_implementation(const std::basic_string<T>& string)
    {
      size_serialization_implementation(string.size());

      for (T character : string)
      {
        data_serialization_implementation(character);
      }
    }
    
    template<typename T>
    void data_deserialization_implementation(std::basic_string<T>& string)
    {
      // std::cout << "STRING DESERIALIZATION\n";
      size_t size;
      size_deserialization_implementation(size);

      string.clear();

      T character;
      for (size_t k = 0; k < size; ++k)
      {
        data_deserialization_implementation(character);
        string += character;
      }
    }
//--------------------------------------------------------------------------------------------------
    template<typename T>
    void data_serialization_implementation(const std::vector<T>& data)
    {
      size_serialization_implementation(data.size());

      for (const T& value : data)
      {
        data_serialization_implementation(value);
      }
    }

    template<typename T>
    void data_deserialization_implementation(std::vector<T>& data)
    {
      // std::cout << "VECTOR DESERIALIZATION\n";
      data.clear();

      size_t size;
      size_deserialization_implementation(size);

      data.reserve(size);

      T value;
      for (size_t k = 0; k < size; ++k)
      {
        data_deserialization_implementation(value);
        data.push_back(value);
      }
    }
//--------------------------------------------------------------------------------------------------
    template<typename T>
    void data_serialization_implementation(const std::list<T>& data)
    {
      size_serialization_implementation(data.size());

      for (const T& value : data)
      {
        data_serialization_implementation(value);
      }
    }

    template<typename T>
    void data_deserialization_implementation(std::list<T>& data)
    {
      // std::cout << "LIST DESERIALIZATION\n";
      data.clear();

      size_t size;
      size_deserialization_implementation(size);

      T value;
      for (size_t k = 0; k < size; ++k)
      {
        data_deserialization_implementation(value);
        data.push_back(value);
      }
    }
//--------------------------------------------------------------------------------------------------
    template<typename T>
    void data_serialization_implementation(const std::deque<T>& data)
    {
      size_serialization_implementation(data.size());

      for (const T& value : data)
      {
        data_serialization_implementation(value);
      }
    }

    template<typename T>
    void data_deserialization_implementation(std::deque<T>& data)
    {
      // std::cout << "DEQUE DESERIALIZATION\n";
      data.clear();

      size_t size;
      size_deserialization_implementation(size);

      T value;
      for (size_t k = 0; k < size; ++k)
      {
        data_deserialization_implementation(value);
        data.push_back(value);
      }
    }
//--------------------------------------------------------------------------------------------------
    template<typename T1, typename T2>
    void data_serialization_implementation(const std::pair<T1, T2>& data)
    {
      data_serialization_implementation(data.first);
      data_serialization_implementation(data.second);
    }

    template<typename T1, typename T2>
    void data_deserialization_implementation(std::pair<T1, T2>& data)
    {
      // std::cout << "PAIR DESERIALIZATION\n";
      data_deserialization_implementation(data.first);
      data_deserialization_implementation(data.second);
    }
//--------------------------------------------------------------------------------------------------
    template<class M>
    void data_serialization_implementation(M& data, enable_if_maplike<M>*)
    {
      size_serialization_implementation(data.size());

      for (const std::pair<typename M::key_type, typename M::mapped_type>& key_value : data)
      {
        data_serialization_implementation(key_value);
      }
    }

    template<class M>
    void data_deserialization_implementation(M& data, enable_if_maplike<M>*)
    {
      // std::cout << "UNORDERED_MAP DESERIALIZATION\n";
      size_t size;
      size_deserialization_implementation(size);

      data.clear();

      std::pair<typename M::key_type, typename M::mapped_type> key_value;
      for (size_t k = 0; k < size; ++k)
      {
        data_deserialization_implementation(key_value);
        data.insert(key_value);
      }
    }
//--------------------------------------------------------------------------------------------------
    template<class S>
    void data_serialization_implementation(const S& data, enable_if_setlike<S>*)
    {
      size_serialization_implementation(data.size());

      for (const typename S::key_type& key : data)
      {
        data_serialization_implementation(key);
      }
    }

    template<class S>
    void data_deserialization_implementation(S& data, enable_if_setlike<S>*)
    {
      // std::cout << "UNORDERED_SET DESERIALIZATION\n";
      size_t size;
      size_deserialization_implementation(size);

      data.clear();

      typename S::key_type key;
      for (size_t k = 0; k < size; ++k)
      {
        data_deserialization_implementation(key);
        data.insert(key);
      }
    }
  }
//--------------------------------------------------------------------------------------------------
  inline namespace Frontend
  {
    void print_bytes(const uint8_t data[], const size_t size, const bool print_header)
    {
      if (print_header)
      {
        Global::out << "bytes[" << std::dec << size << "]: ";
      }

      Global::out << std::hex<< std::setfill('0');
      for (size_t k = 0; k < size; ++k)
      {
        Global::out << std::setw(2) << std::uppercase << unsigned(data[k] & 0xFF) << ' ';
      }

      Global::out << std::endl;
    }
  }
//--------------------------------------------------------------------------------------------------
}
#endif