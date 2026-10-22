#ifndef OXYIR_UTILITIES_H
#define OXYIR_UTILITIES_H

#include <string_view>
#include <meta>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>

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

    inline std::string to_lower(std::string_view string_view) {
        std::string result(string_view);
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return result;
    }

    inline std::string type_to_string(Type * actual_type) {
        if (!actual_type) {
            return "<null>";
        }

        if (auto primitive_type = dynamic_cast<PrimitiveType*>(actual_type)) {
            return to_lower(enum_to_string(primitive_type->getPrimitiveKind()));
        }

        if (auto pointer_type = dynamic_cast<PointerType*>(actual_type)) {
            return "ptr";
        }

        if (auto array_type = dynamic_cast<ArrayType*>(actual_type)) {
            return "[" + type_to_string(array_type->getElementType()) + ", " + std::to_string(array_type->getSize()) + "]";
        }

        if (auto struct_type = dynamic_cast<StructType*>(actual_type)) {
            std::string result = "struct { ";
            bool first = true;
            for (const auto& member_type : struct_type->getMemberTypes()) {
                if (!first) {
                    result += ", ";
                }
                result += type_to_string(member_type);
                first = false;
            }
            result += " }";
            return result;
        }

        if (auto function_type = dynamic_cast<FunctionType*>(actual_type)) {
            std::string result = "func (";
            bool first = true;
            for (const auto& param_type : function_type->getParamTypes()) {
                if (!first) {
                    result += ", ";
                }
                result += type_to_string(param_type);
                first = false;
            }
            if (function_type->isVarArg()) {
                if (!first) {
                    result += ", ";
                }
                result += "...";
            }
            result += ") -> " + type_to_string(function_type->getReturnType());
            return result;
        }

        if (dynamic_cast<VariadicListType*>(actual_type)) {
            return "va_list";
        }

        if (dynamic_cast<VoidType*>(actual_type)) {
            return "void";
        }

        if (dynamic_cast<TypeValue*>(actual_type)) {
            return "type";
        }

        if (dynamic_cast<CustomInstructionSet*>(actual_type)) {
            return "custom_instruction_set";
        }

        return "<unknown>";
    }
}

template <typename E>
requires std::is_enum_v<E>
std::ostream& operator<<(std::ostream& os, E value) {
    os << Oxy::Utilities::enum_to_string(value);
    return os;
}

#endif //OXYIR_UTILITIES_H
