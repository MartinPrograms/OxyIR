#ifndef OXYIR_UTILITIES_H
#define OXYIR_UTILITIES_H

#include <string_view>
#include <meta>
#include <iostream>

namespace Oxy::Utilities {
    template<typename E> requires std::is_enum_v<E>
    auto enum_to_string(E value) {
        constexpr bool Enumerable = std::meta::is_enumerable_type(^^E);
        if constexpr (Enumerable) {
            template for (constexpr auto e :
              std::define_static_array(std::meta::enumerators_of(^^E))) {
                if (value == [:e:])
                    return std::meta::identifier_of(e);
            }
        }

        return std::string_view("<unknown>");
    }
}

template <typename E>
requires std::is_enum_v<E>
std::ostream& operator<<(std::ostream& os, E value) {
    os << Oxy::Utilities::enum_to_string(value);
    return os;
}

#endif //OXYIR_UTILITIES_H
