#include <iostream>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>
#include <type_traits>

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

template <typename T>
struct is_generic : std::conditional<
  is_map_like<T>::value ||
  is_set_like<T>::value,
  std::false_type, 
  std::true_type
>::type {};

template<typename T>
using enable_if_generic = typename std::enable_if<is_generic<T>::value>::type;

int main() {
    // using type = std::map<int, double>;
    // using type = std::set<int>;
    using type = std::vector<int>;

    if (is_map_like<type>::value) std::cout << "map-like\n";

    if (is_set_like<type>::value) std::cout << "set-like\n";
    
    if (is_generic<type>::value)  std::cout << "generic\n";
    
    return 0;
}
