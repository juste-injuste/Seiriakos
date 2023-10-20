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
    //*
    template <typename T>
    struct is_container
    {
      private:
        template <typename C>
        static auto test_size(int) -> decltype(std::declval<C>().size(), std::true_type{});
        template <typename>
        static auto test_size(...) -> std::false_type;

        template <typename C>
        static auto test_begin(int) -> decltype(std::declval<C>().begin(), std::true_type{});
        template <typename>
        static auto test_begin(...) -> std::false_type;

        template <typename C>
        static auto test_end(int) -> decltype(std::declval<C>().end(), std::true_type{});
        template <typename>
        static auto test_end(...) -> std::false_type;
      public:
        static const bool value = {
          decltype(test_size<T>(0))::value
          and decltype(test_begin<T>(0))::value
          and decltype(test_end<T>(0))::value
        };
    };

    template<typename T>
    using enable_if_container = typename std::enable_if<is_container<T>::value>::type;

    template<typename T>
    using is_Serializable = std::is_base_of<Serializable, T>;

    template<typename T>
    using enable_if_generic = typename std::enable_if<not is_Serializable<T>::value and not is_container<T>::value>::type;

    // template<typename T>
    // std::vector<uint8_t> data_serializization_implementation(const T& data)
    // {
      
    //   return std::vector<uint8_t>{};
    // }

    // template<typename T>
    // std::vector<uint8_t> data_serializization_implementation(const T& data)
    // {
      
    //   return std::vector<uint8_t>{};
    // }

    // template<typename T>
    // std::vector<uint8_t> data_serializization_implementation(const T& data)
    // {
      
    //   return std::vector<uint8_t>{};
    // }

    // template<typename T>
    // std::vector<uint8_t> data_serializization_implementation(const T& data)
    // {
      
    //   return std::vector<uint8_t>{};
    // }
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

        // serialize/deserialize generic data type
        template<typename T, typename = Backend::enable_if_generic<T>>
        inline void data_serialization(const T& data) const noexcept;
        template<typename T, typename = Backend::enable_if_generic<T>>
        inline void data_deserialization(T& data) noexcept;
        
        // serialize/deserialize Serializable data type
        inline void data_serialization(const Serializable& data) const noexcept;
        inline void data_deserialization(Serializable& data) noexcept;

        // recursive calls to appropriate data_serialization/data_deserialization overloads
        template<typename T1, typename T2, typename... T>
        inline void data_serialization(const T1& data_1, const T2& data_2, const T&... data_n) const noexcept;
        inline void data_serialization() const noexcept;
        template<typename T1, typename T2, typename... T>
        inline void data_deserialization(T1& data_1, T2& data_2, T&... data_n) noexcept;
        inline void data_deserialization() noexcept;
      private:
        // issue error message
        inline void error(const char* message) const noexcept;
        #ifdef SEIRIAKOS_THREADSAFE
          // thread-local error flag
          static thread_local bool error_flag;
          // thread-local byte buffer
          static thread_local std::vector<uint8_t> buffer;
          // thread-local index to the front of the buffer
          static thread_local size_t front_of_buffer;
        #else 
          // error flag
          static bool error_flag;
          // byte buffer
          static std::vector<uint8_t> buffer;
          // index to the front of the buffer
          static size_t front_of_buffer;
        #endif
    };

    // Serializable static member initializations
    #ifdef SEIRIAKOS_THREADSAFE
      thread_local bool Serializable::error_flag;
      thread_local std::vector<uint8_t> Serializable::buffer;
      thread_local size_t Serializable::front_of_buffer;
    #else
      bool Serializable::error_flag;
      std::vector<uint8_t> Serializable::buffer;
      size_t Serializable::front_of_buffer;
    #endif
  }
//--------------------------------------------------------------------------------------------------
  inline namespace Frontend
  {
    std::vector<uint8_t> Serializable::serialize() const noexcept
    {
      buffer.clear(); // initialize buffer

      // call the implementation provided by the inheriting class
      serialization_sequence();

      return buffer; // return serialized data
    }

    bool Serializable::deserialize(const uint8_t data[], const size_t size) noexcept
    {
      buffer.assign(data, data + size); // initialize buffer with data
      front_of_buffer = 0;              // initialize front of buffer
      error_flag      = false;          // initialize error flag

      // call the implementation provided by the inheriting class
      deserialization_sequence();
      
      if (front_of_buffer != buffer.size())
      {
        error("buffer is not empty, serialization/deserialization sequence mismatch");
      }

      return error_flag;
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

    void Serializable::data_serialization(const Serializable& data) const noexcept
    {
      data.serialization_sequence();   // serialize data via its specialized implementation
    }

    void Serializable::data_deserialization(Serializable& data) noexcept
    {
      data.deserialization_sequence(); // deserialize data via its specialized implementation
    }

    template<typename T1, typename T2, typename... T>
    void Serializable::data_serialization(const T1& data_1, const T2& data_2, const T&... data_n) const noexcept
    {
      data_serialization(data_1);
      data_serialization(data_2);
      data_serialization(data_n...);
    }

    void Serializable::data_serialization() const noexcept
    {}

    template<typename T1, typename T2, typename... T>
    void Serializable::data_deserialization(T1& data_1, T2& data_2, T&... data_n) noexcept
    {
      data_deserialization(data_1);
      data_deserialization(data_2);
      data_deserialization(data_n...);
    }

    void Serializable::data_deserialization() noexcept
    {}

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