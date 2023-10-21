#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <type_traits>
#include <vector>

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

template<class M>
using enable_if_maplike = typename std::enable_if<is_map_like<M>::value>::type;

template<class S>
using enable_if_setlike = typename std::enable_if<is_set_like<S>::value>::type;

class Serializable;

template<typename T>
using is_Serializable = std::is_base_of<Serializable, T>;

template <typename T>
struct is_generic : std::conditional<
  is_Serializable<T>::value ||
  is_map_like<T>::value ||
  is_set_like<T>::value,
  std::false_type, 
  std::true_type
>::type {};

template<typename T>
using enable_if_generic = typename std::enable_if<is_generic<T>::value>::type;

// Implementation for map-like containers (map, multimap, unordered_map, unordered_multimap)
template <class M>
void data_deserialization_implementation(M& data, enable_if_maplike<M>* = nullptr)
{
    std::cout << "Map-like DESERIALIZATION\n";
    // Your implementation for map-like types
}

// Implementation for set-like containers (set, multiset, unordered_set, unordered_multiset)
template <class S>
void data_deserialization_implementation(S& data, enable_if_setlike<S>* = nullptr)
{
    std::cout << "Set-like DESERIALIZATION\n";
    // Your implementation for set-like types
}

// Generic implementation for other types
template <typename T, typename = enable_if_generic<T>>
void data_deserialization_implementation(T& data)
{
    std::cout << "Unsupported type\n";
    // Your implementation for other types
}

int main()
{
    std::map<int, int> map;
    std::multimap<int, int> multimap;
    std::unordered_map<int, int> unordered_map;
    std::unordered_multimap<int, int> unordered_multimap;
    std::set<int> set;
    std::multiset<int> multiset;
    std::unordered_set<int> unordered_set;
    std::unordered_multiset<int> unordered_multiset;
    std::vector<int> vector;


    auto t = is_generic<std::vector<int>>::value;

    data_deserialization_implementation(map); // Calls the map-like version
    data_deserialization_implementation(multimap); // Calls the map-like version
    data_deserialization_implementation(unordered_map); // Calls the map-like version
    data_deserialization_implementation(unordered_multimap); // Calls the map-like version
    data_deserialization_implementation(set); // Calls the set-like version
    data_deserialization_implementation(multiset); // Calls the set-like version
    data_deserialization_implementation(unordered_set); // Calls the set-like version
    data_deserialization_implementation(unordered_multiset); // Calls the set-like version
    data_deserialization_implementation(vector); // Calls the default "Unsupported type" version

    return 0;
}
