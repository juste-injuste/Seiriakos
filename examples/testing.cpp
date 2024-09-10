#include <cstddef>
#include <iostream>

constexpr size_t good(size_t k, size_t N) { return k < (N - 1); }
constexpr size_t next(size_t k, size_t N) { return good(k, N) ? (k + 1) : (N - 1); }

template<size_t N, size_t k = 0>
constexpr
bool has_character(const char (&string)[N], const char character)
{
  return good(k, N) ? 
    string[k] == character || has_character<N, next(k, N)>(string, character)
  : false;
}

template<size_t N1, size_t N2, size_t k1 = 0>
constexpr
bool starts_with(const char (&string)[N1], const char (&word)[N2])
{
  return true;//string[k] == '?' || (k < N-1 ? starts_with<N, next(k, N)>(string) : false);
}

template<size_t N1, size_t k1 = 0, size_t ABD, size_t CBD, size_t SBD>
constexpr
bool is_typename(const char (&string)[N1])
{
  return true;
}

template<size_t N, size_t k = 0>
constexpr
bool is_base(const char (&string)[N])
{
  return starts_with<N, k>(string, "seiriakos::")
    ||   starts_with<N, k>(string, "stz::seiriakos::")
    ||   starts_with<N, k>(string, "stz::")
    ||   starts_with<N, k>(string, "base_type")
    ? is_typename<N, k>(string) // good
    : false; // is not
}

constexpr
bool is_allowed(const char character)
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
bool is_mutable(const char (&string)[N1])
{
  return true;
}

template<size_t N, size_t k = 0>
constexpr
bool is_variable_list(const char (&string)[N])
{
  return good(k, N) ? true
    : is_variable_list<N, next(k, N)>(string) and
      is_allowed(string[k]) ? true
    : is_base<N, k>(string) or is_mutable<N, k>(string);
}

template<size_t N, size_t k = 0>
constexpr
size_t find_character(const char (&string)[N], const char character)
{
  constexpr auto NOT_FOUND = static_cast<size_t>(-1);

  return good(k, N) ? NOT_FOUND
    : string[k] == character ? k
    : find_character<N, next(k, N)>(string, character);
}

template<size_t N1>
constexpr
size_t get_n(const char (&string)[N1], const size_t k)
{
  return k < N1 ? string[k] : ' ';
}

constexpr
bool was_found(const size_t k)
{
  return k != static_cast<size_t>(-1);
}

template<size_t N1, size_t N2, size_t k1 = 0, size_t k2 = 0, bool in_string = false>
constexpr
bool contains_word(const char (&string)[N1], const char (&word)[N2])
{
  return good(k1, N1) ? // go on
    in_string ?
      (string[k1] == '\\') ? // escape sequence encountered
        contains_word<N1, N2, next(next(k1, N1), N1), 0, true>(string, word) // skip it
      : (string[k1] == '"') ? // end of string encountered
          contains_word<N1, N2, next(k1, N1), 0, false>(string, word) // no longer in string
        : contains_word<N1, N2, next(k1, N1), 0, true >(string, word) // still in string
    : (string[k1] == word[k2]) and contains_word<N1, N2, next(k1, N1), next(k2, N2)>(string, word)
  : false;
}

int main()
{
  constexpr char string1[] = "find the ?";
  constexpr char string2[] = "find ? the";
  constexpr char string3[] = "find the @";
  static_assert(has_character(string1, '?') == true , "'?' containted in string.");
  static_assert(has_character(string2, '?') == true , "'?' containted in string.");
  static_assert(has_character(string3, '?') == false, "'?' containted in string.");


  constexpr auto test = contains_word(string1, "the");
}