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
#include <cstdint>     // for uint8_t
#include <vector>      // for std::vector
#include <type_traits> // for std::is_base_of
#include <iostream>    // for std::cout, std::cerr
#include <iomanip>     // for std::setw, std::setfill, std::hex
#include <ios>         // for std::uppercase
//---Seiriakos library------------------------------------------------------------------------------
namespace Seiriakos
{
  namespace Version
  {
    const long MAJOR = 000;
    const long MINOR = 001;
    const long PATCH = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
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

    // output ostream
    std::ostream out_ostream{std::cout.rdbuf()};

    // error ostream
    std::ostream err_ostream{std::cerr.rdbuf()};
  }
//--------------------------------------------------------------------------------------------------
  namespace Backend
  { 
    template <typename T>
    struct has_const_iterator
    {
      private:
        template <typename U>
        static decltype(std::declval<U>().cbegin(), std::declval<U>().cend(), std::true_type{}) test(int);

        template <typename>
        static std::false_type test(...);
      public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    using is_Serializable = std::is_base_of<Serializable, T>;

    template <typename T>
    struct is_generic : std::conditional<
      std::is_base_of<Serializable, T>::value || has_const_iterator<T>::value,
      std::false_type,
      std::true_type
    >::type {};

    template<typename T>
    using enable_if_Serializable = typename std::enable_if<is_Serializable<T>::value>::type;

    template<typename T>
    using enable_if_only_const_iterable = typename std::enable_if<has_const_iterator<T>::value&&!is_Serializable<T>::value>::type;

    template<typename T>
    using enable_if_generic = typename std::enable_if<is_generic<T>::value>::type;
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
        inline void deserialize(const uint8_t data[], const size_t size) noexcept;
      private:
      protected:
        // serialization sequence (provided by the inheriting class)
        virtual void serialization_sequence() const noexcept = 0;
        // deserialization sequence (provided by the inheriting class)
        virtual void deserialization_sequence() noexcept = 0;

        // serialize generic data type
        template<typename T, typename = Backend::enable_if_generic<T>>
        inline void data_serialization(const T& data) const noexcept;
        
        // serialize Serializable data type
        inline void data_serialization(const Serializable& data) const noexcept;

        // recursive calls to appropriate data_serialization overloads
        template<typename T1, typename T2, typename... T>
        inline void data_serialization(const T1& data1, const T2& data2, const T&... datas) const noexcept;
        inline void data_serialization() const noexcept {}

        // deserialize generic data type
        template<typename T, typename = Backend::enable_if_generic<T>>
        inline void data_deserialization(T& data) noexcept;

        // deserialize Serializable data type
        inline void data_deserialization(Serializable& data) noexcept;

        // recursive calls to appropriate data_deserialization overloads
        template<typename T1, typename T2, typename... T>
        inline void data_deserialization(T1& data1, T2& data2, T&... datas) noexcept;
        inline void data_deserialization() noexcept {}
      private:
        // issue error message
        inline void error(const char* message) const noexcept;
        #ifdef SEIRIAKOS_THREADSAFE
          // thread-local byte buffer
          static thread_local std::vector<uint8_t> buffer;
          // thread-local index to the front of the buffer
          static thread_local size_t front_of_buffer;
        #else
          // byte buffer
          static std::vector<uint8_t> buffer;
          // index to the front of the buffer
          static size_t front_of_buffer;
        #endif
    };

    // Serializable static member initializations
    #ifdef SEIRIAKOS_THREADSAFE
      thread_local std::vector<uint8_t> Serializable::buffer;
      thread_local size_t Serializable::front_of_buffer;
    #else
      std::vector<uint8_t> Serializable::buffer;
      size_t Serializable::front_of_buffer;
    #endif
  }
//--------------------------------------------------------------------------------------------------
  inline namespace Frontend
  {
    std::vector<uint8_t> Serializable::serialize() const noexcept
    {
      buffer.clear();           // empty buffer
      serialization_sequence(); // call the implementation provided by the inheriting class
      return buffer;            // return serialized data
    }

    void Serializable::deserialize(const uint8_t data[], const size_t size) noexcept
    {
      buffer.assign(data, data + size); // fill buffer with data
      front_of_buffer = 0;              // initialize front of buffer
      deserialization_sequence();       // call the implementation provided by the inheriting class
      if (front_of_buffer != buffer.size())
      {
        error("buffer is not empty, serialization/deserialization sequence mismatch");
      }
    }

    template<typename T, typename>
    void Serializable::data_serialization(const T& data) const noexcept
    {
      // add data's bytes one by one to the buffer
      for (size_t k = 0; k < sizeof(T); ++k)
      {
        buffer.push_back(reinterpret_cast<const uint8_t*>(&data)[k]);
      }
    }

    void Serializable::data_serialization(const Serializable& data) const noexcept
    {
      data.serialization_sequence(); // serialize data via its implementation
    }

    template<typename T1, typename T2, typename... T>
    void Serializable::data_serialization(const T1& data1, const T2& data2, const T&... datas) const noexcept
    {
      data_serialization(data1);
      data_serialization(data2);
      data_serialization(datas...);
    }
    void Serializable::data_deserialization(Serializable& data) noexcept
    {
      data.deserialization_sequence(); // deserialize data via its implementation
    }

    template<typename T, typename>
    void Serializable::data_deserialization(T& data) noexcept
    {
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

    template<typename T1, typename T2, typename... T>
    void Serializable::data_deserialization(T1& data1, T2& data2, T&... datas) noexcept
    {
      data_deserialization(data1);
      data_deserialization(data2);
      data_deserialization(datas...);
    }

    void Serializable::error(const char* message) const noexcept
    {
      err_ostream << "error: Serializable: " << message << std::endl;
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
  inline namespace Frontend
  {
    void print_bytes(const uint8_t data[], const size_t size, const bool print_header)
    {
      if (print_header)
      {
        out_ostream << "bytes[" << std::dec << size << "]: ";
      }

      out_ostream << std::hex<< std::setfill('0');
      for (size_t k = 0; k < size; ++k)
      {
        out_ostream << std::setw(2) << std::uppercase << unsigned(data[k] & 0xFF) << ' ';
      }

      out_ostream << std::endl;
    }
  }
//--------------------------------------------------------------------------------------------------
}
#endif