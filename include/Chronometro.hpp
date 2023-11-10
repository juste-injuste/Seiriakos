/*---author-------------------------------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Chronometro

-----licence------------------------------------------------------------------------------------------------------------
 
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
 
-----versions-----------------------------------------------------------------------------------------------------------

Version 0.1.0 - Initial release

-----description--------------------------------------------------------------------------------------------------------

Chronometro is a simple and lightweight C++11 (and newer) library that allows you to measure the
execution time of functions or code blocks. See the included README.MD file for more information.

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#ifndef CHRONOMETRO_HPP
#define CHRONOMETRO_HPP
// --necessary standard libraries---------------------------------------------------------------------------------------
#include <chrono>   // for clocks and time representations
#include <iostream> // for std::cout, std::cerr, std::endl
#include <cstddef>  // for size_t
#include <ostream>  // for std::ostream
#include <string>   // for std::string
// --Chronometro library------------------------------------------------------------------------------------------------
namespace Chronometro
{
  namespace Version
  {
    const long MAJOR = 000;
    const long MINOR = 001;
    const long PATCH = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }

  namespace Global
  {
    std::ostream out{std::cout.rdbuf()}; // output ostream
    std::ostream err{std::cerr.rdbuf()}; // error ostream
    std::ostream wrn{std::cerr.rdbuf()}; // warning ostream
  }
// --Chronometro library : frontend forward declarations----------------------------------------------------------------
  inline namespace Frontend
  {
    // bring clocks and nanoseconds to frontend
    using std::chrono::system_clock;
    using std::chrono::steady_clock;
    using std::chrono::high_resolution_clock;
    using std::chrono::nanoseconds;

    // time units for displaying
    // enum class Unit : unsigned char
    // {
    //   ns,       // nanoseconds
    //   us,       // microseconds
    //   ms,       // milliseconds
    //   s,        // seconds
    //   min,      // minutes
    //   h,        // hours
    //   automatic // deduce the appropriate unit
    // };

    // class Result final
    // {
    //   public:
    //     inline explicit Result(const char* format, const nanoseconds::rep nanoseconds) noexcept;
    //     nanoseconds::rep nanoseconds;
    //     const char* format;
    //     inline std::ostream& print(std::ostream& ostream = Global::out) const noexcept;
    // };

    struct Time
    {
      nanoseconds::rep nanoseconds;
    };

    std::ostream& operator << (std::ostream& ostream, const Time time) noexcept;

    // measure elapsed time
    template<typename C = high_resolution_clock>
    class Stopwatch final
    {
      public:
        using Clock = C;
        // start measuring time
        inline explicit Stopwatch() noexcept;
        // display and return lap time
        // Result lap(const char* asset = "lap time: ") noexcept;
        Time lap() noexcept;
        // display and return split time
        // Result split(const char* asset = "elapsed time: ") noexcept;
        Time split() noexcept;
        // pause time measurement
        void pause() noexcept;
        // reset measured times
        void reset() noexcept;
        // unpause time measurement
        void unpause() noexcept;
        // unit to be used when displaying elapsed time
      private:
        // issue warning
        inline void warning(const char* message) const noexcept;
        // issue error
        inline void error(const char* message) const noexcept;
        // used to keep track of the current status
        bool is_paused;
        // measured elapsed times
        typename C::duration duration;
        // measured elapsed time
        typename C::duration duration_lap;
        // time either at construction or from last unpause
        typename C::time_point previous;
        // time either at construction or from last unpause/lap
        typename C::time_point previous_lap;
    };

    class Measure final
    {
      public:
        inline Measure() noexcept;
        inline Measure(size_t n) noexcept;
        inline Measure(size_t n, const char* format) noexcept;
        inline size_t operator*() noexcept;
        inline void operator++() noexcept;
        inline bool operator!=(const Measure&) noexcept;
        inline operator bool() noexcept;
        inline Measure& begin() noexcept;
        inline Measure end() noexcept;
      private:
        Stopwatch<>      stopwatch;

        const size_t     iterations;
        size_t           iterations_left;

        const char* lap_format = nullptr;
        Time        lap_time;
    };

    // measure function execution time
    template<typename C = high_resolution_clock, typename F, typename... A>
    typename C::duration execution_time(const F function, const size_t repetitions, const A... arguments);

    // measure function execution time without function calling via pointers
    #define CHRONOMETRO_EXECUTION_TIME(function, repetitions, ...)

    // repeats the following statement/block n times
    #define CHRONOMETRO_REPEAT(n)

    // measures the time it takes to execute the following statement/block n times
    #define CHRONOMETRO_MEASURE(n)
  }
// --Chronometro library : backend forward declaration------------------------------------------------------------------
  namespace Backend
  {
    // returns the appropriate unit to display time
    const char* appropriate_unit(const Time time) noexcept;

    const char* format_string(Time time, std::string format, size_t iteration = 0) noexcept;
  }
// --Chronometro library : frontend definitions-------------------------------------------------------------------------
  inline namespace Frontend
  {
    // Result::Result(const char* format, const nanoseconds::rep nanoseconds) noexcept :
    //   nanoseconds(nanoseconds),
    //   format(format)
    // {}

    // std::ostream& Result::print(std::ostream& ostream) const noexcept
    // {
    //   // if unit_ == automatic, deduce the appropriate unit
    //   switch ((unit == Unit::automatic) ? Backend::appropriate_unit(nanoseconds) : unit)
    //   {
    //     // case Unit::ns:
    //     //   return ostream << asset << nanoseconds << " ns";
    //     // case Unit::us:
    //     //   return ostream << asset << nanoseconds / 1000 << " us";
    //     // case Unit::ms:
    //     //   return ostream << asset << nanoseconds / 1000000 << " ms";
    //     // case Unit::s:
    //     //   return ostream << asset << nanoseconds / 1000000000 << " s";
    //     // case Unit::min:
    //     //   return ostream << asset << nanoseconds / 60000000000 << " min";
    //     // case Unit::h:
    //     //   return ostream << asset << nanoseconds / 3600000000000 << " h";
    //     default:
    //       Global::err << "error: Result: print: invalid unit, invalid code path reached" << std::endl;
    //       return ostream;
    //   }
    // }

    std::ostream& operator << (std::ostream& ostream, const Time time) noexcept
    {
      // 10 h < duration
      if (time.nanoseconds > 36000000000000)
      {
        return ostream << "elapsed time: " << time.nanoseconds << std::endl;//Unit::h;
      }

      // 10 min < duration <= 10 h
      if (time.nanoseconds > 600000000000)
      {
        return ostream << "elapsed time: %min" << time.nanoseconds << std::endl;//Unit::min;
      }

      // 10 s < duration <= 10 m
      if (time.nanoseconds > 10000000000)
      {
        return ostream << Backend::format_string(time, "elapsed time: %s") << std::endl;//Unit::s;
      }

      // 10 ms < duration <= 10 s
      if (time.nanoseconds > 10000000)
      {
        return ostream << Backend::format_string(time, "elapsed time: %ms") << std::endl;//Unit::ms;
      }

      // 10 us < duration <= 10 ms
      if (time.nanoseconds > 10000)
      {
        return ostream << Backend::format_string(time, "elapsed time: %us") << std::endl;//Unit::us;
      }

      // duration <= 10 us
      return ostream << Backend::format_string(time, "elapsed time: %us") << std::endl;//Unit::ns;
    }

    template<typename C>
    Stopwatch<C>::Stopwatch() noexcept :
      is_paused(false),
      duration{},
      duration_lap{},
      previous(C::now()),
      previous_lap(previous)
    {}

    template<typename C>
    Time Stopwatch<C>::lap() noexcept
    {
      // measure current time
      const typename C::time_point now = C::now();

      nanoseconds::rep ns = duration_lap.count();

      if (is_paused == false)
      {
        // save elapsed times
        duration += now - previous;
        ns       += (now - previous_lap).count();
        
        // reset measured time
        duration_lap = typename C::duration{};
        previous     = C::now();
        previous_lap = previous;
      }
      else warning("cannot measure lap, must not be paused");

      return Time{ns};
    }

    template<typename C>
    Time Stopwatch<C>::split() noexcept
    {
      // measure current time
      const typename C::time_point now = C::now();

      nanoseconds::rep ns = duration.count();

      if (is_paused == false)
      {
        // save elapsed times
        duration     += now - previous;
        duration_lap += now - previous_lap;
        
        ns = duration.count();

        // save time point
        previous     = C::now();
        previous_lap = previous;
      }
      else warning("cannot measure split, must not be paused");
      
      // return result;
      return Time{ns};
    }

    template<typename C>
    void Stopwatch<C>::pause() noexcept
    {
      // measure current time
      const typename C::time_point now = C::now();

      // add elapsed time up to now if not paused
      if (is_paused == false)
      {
        // pause
        is_paused = true;

        // save elapsed times
        duration     += now - previous;
        duration_lap += now - previous_lap;
      }
      else warning("cannot pause further, is already paused");
    }

    template<typename C>
    void Stopwatch<C>::reset() noexcept
    {
      // reset measured time
      duration     = typename C::duration{};
      duration_lap = typename C::duration{};

      // hot reset if unpaused
      if (is_paused == false)
      {
        previous     = C::now();
        previous_lap = previous;
      } 
    }

    template<typename C>
    void Stopwatch<C>::unpause() noexcept
    {
      if (is_paused == true)
      {
        // unpause
        is_paused = false;

        // reset measured time
        previous     = C::now();
        previous_lap = previous;
      }
      else warning("is already unpaused");
    }

    template<typename C>
    void Stopwatch<C>::warning(const char* message) const noexcept
    {
      Global::wrn << "warning: Stopwatch: " << message << std::endl;
    }

    template<typename C>
    void Stopwatch<C>::error(const char* message) const noexcept
    {
      Global::err << "error: Stopwatch: " << message << std::endl;
    }
    
    Measure::Measure() noexcept :
      iterations(1),
      iterations_left(1)
    {}
    
    Measure::Measure(size_t n) noexcept :
      iterations(n),
      iterations_left(n)
    {}
    
    Measure::Measure(size_t n, const char* format) noexcept :
      iterations(n),
      iterations_left(n),
      lap_format{format}
    {}
    
    size_t Measure::operator*() noexcept
    {
      return iterations - iterations_left;
    }

    void Measure::operator++() noexcept
    {
      lap_time = stopwatch.lap();
      
      stopwatch.pause();

      if (lap_format)
      {
        Global::out << Backend::format_string(lap_time, lap_format, iterations - iterations_left) << std::endl;
      }

      --iterations_left;

      stopwatch.unpause();
    }

    bool Measure::operator!=(const Measure&) noexcept
    {
      return bool(*this);
    }

    Measure::operator bool() noexcept
    {
      if (iterations_left)
      {
        return true;
      }

      Global::out << Backend::format_string(stopwatch.split(), "elapsed time: %ms") << std::endl;

      return false;
    }

    Measure& Measure::begin() noexcept
    {
      return *this;
    }

    Measure Measure::end() noexcept
    {
      return Measure{0};
    }

    template<typename C, typename F, typename... A>
    typename C::duration execution_time(const F function, const size_t repetitions, const A... arguments)
    {
      Stopwatch<C> stopwatch;

      for (size_t iteration = 0; iteration < repetitions; ++iteration)
      {
        function(arguments...);
      }

      return stopwatch.split();
    }

    #undef  CHRONOMETRO_EXECUTION_TIME
    #define CHRONOMETRO_EXECUTION_TIME(function, n, ...)             \
      [&]() -> Chronometro::high_resolution_clock::duration          \
      {                                                              \
        Chronometro::Stopwatch<> sto_pw_atch;                        \
        for (size_t repet_itio_ns_lef_t = n; repet_itio_ns_lef_t--;) \
        {                                                            \
          function(__VA_ARGS__);                                     \
        }                                                            \
        return sto_pw_atch.split();                                  \
      }()

    #undef  CHRONOMETRO_REPEAT
    #define CHRONOMETRO_REPEAT(n)  for (size_t repet_itio_ns_lef_t = n; repet_itio_ns_lef_t--;)

    #undef  CHRONOMETRO_MEASURE
    #define CHRONOMETRO_MEASURE(n) for (Chronometro::Measure wo_r_k{n}; wo_r_k; ++wo_r_k)

    #undef  CHRONOMETRO_MEASURE_LAPS
    #define CHRONOMETRO_MEASURE_LAPS(n, format) for (Chronometro::Measure wo_r_k{n, format}; wo_r_k; ++wo_r_k)
  }
// --Chronometro library : backend definitions--------------------------------------------------------------------------
  namespace Backend
  {
    const char* appropriate_unit(const Time time) noexcept
    {
      // 10 h < duration
      if (time.nanoseconds > 36000000000000)
      {
        return "%h";//Unit::h;
      }

      // 10 min < duration <= 10 h
      if (time.nanoseconds > 600000000000)
      {
        return "%min";//Unit::min;
      }

      // 10 s < duration <= 10 m
      if (time.nanoseconds > 10000000000)
      {
        return "%s";//Unit::s;
      }

      // 10 ms < duration <= 10 s
      if (time.nanoseconds > 10000000)
      {
        return "%ms";//Unit::ms;
      }

      // 10 us < duration <= 10 ms
      if (time.nanoseconds > 10000)
      {
        return "%us";//Unit::us;
      }

      // duration <= 10 us
      return "%us";//Unit::ns;
    }

    const char* format_string(Time time, std::string format, size_t iteration) noexcept
    {
      size_t iteration_position = format.find("%#");
      if (iteration_position != std::string::npos)
      {
        format.replace(iteration_position, 2, std::to_string(iteration));
      }

      size_t time_position = format.rfind("%ms");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/1000000) + ' ');
        return format.c_str();
      }

      time_position = format.rfind("%us");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/1000) + ' ');
        return format.c_str();
      }

      time_position = format.rfind("%us");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds) + ' ');
        return format.c_str();
      }

      time_position = format.rfind("%s");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/1000000000) + ' ');
        return format.c_str();
      }

      time_position = format.rfind("%min");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/60000000) + ' ');
        return format.c_str();
      }

      time_position = format.rfind("%h");
      if (time_position != std::string::npos)
      {
        format.replace(time_position, 1, std::to_string(time.nanoseconds/3600000000) + ' ');
        return format.c_str();
      }
      
      return format.c_str();
    }
  }
}
#endif
