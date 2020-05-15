#ifndef INCLUDE_VARIABLES_H_
#define INCLUDE_VARIABLES_H_

#include <map>

template<typename V>
class Variables final {
    typedef char K;

    std::map<K, V> const values_;

public:

    explicit Variables(std::map<K, V> const& values) : values_(values) {}

    explicit Variables(std::map<K, V>&& values) : values_(values) {}

    std::optional<V> get(K const& name) {
        auto const values = values_;
        auto const iterator = values.find(name);

        return iterator == values.end() ? std::optional<V>() : std::optional<V>(iterator->second);
    }
};


#endif //INCLUDE_VARIABLES_H_
