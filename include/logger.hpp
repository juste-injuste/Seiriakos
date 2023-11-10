/*---author-----------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/

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

-----inclusion guard------------------------------------------------------------------------------*/
#ifndef LOGGER_H
#define LOGGER_H
//---necessary standard libraries-------------------------------------------------------------------
#include <cstddef>     // for size_t
#include <string>
#include <ostream>
#include <iostream>
#define LOGGING
//---Seiriakos library------------------------------------------------------------------------------
namespace Logger
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
    std::ostream log{std::clog.rdbuf()}; // logging ostream
    std::ostream err{std::cerr.rdbuf()}; // error ostream
    std::ostream wrn{std::cerr.rdbuf()}; // warning ostream
  }
//----------------------------------------------------------------------------------------------------------------------
# ifdef LOGGING
  namespace Backend
  {
# if defined(__GNUC__)
    inline std::string get_function_name(std::string&& full_name)
    {
      const size_t start = full_name.find(' ') + 1;
      const size_t end   = full_name.find('(') - start;
      return full_name.substr(start, end) + ": ";
    }
#   define LOGGER_FUNCTION_NAME Logger::Backend::get_function_name(__PRETTY_FUNCTION__)
# else
#   define LOGGER_FUNCTION_NAME (std::string{__func__} + ": ")
# endif

    class IndentedLog
    {
    public:
      inline IndentedLog(const char* text, const std::string& caller = "") noexcept
      {
        for (size_t k = indentation; k; --k)
        {
          Global::log << ' ';
        }

        Global::log << "log: " << caller << text << std::endl;

        indentation += 2;
      }

      inline ~IndentedLog() noexcept
      {
        indentation -= 2;
      }
    private:
      static size_t indentation;
    };
    size_t IndentedLog::indentation;
  }
//----------------------------------------------------------------------------------------------------------------------
# define LOGGER_LOG(text)  Global::log << "log: " << LOGGER_FUNCTION_NAME << text << std::endl;
# define LOGGER_ILOG(text) Logger::Backend::IndentedLog inde_nt_ed_l_og_{text, LOGGER_FUNCTION_NAME};
#else
# define LOGGER_LOG(text)  /* to enable logging #define LOGGING */
# define LOGGER_ILOG(text) /* to enable logging #define LOGGING */
#endif
//----------------------------------------------------------------------------------------------------------------------
}
#endif