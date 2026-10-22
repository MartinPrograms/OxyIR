#ifndef OXYIR_VARIADICLISTTYPE_H
#define OXYIR_VARIADICLISTTYPE_H

#include "Type.h"
#include <string>

// Opaque type representing a variadic list of arguments.

namespace Oxy {
    class VariadicListType : public Type {
    public:
        explicit VariadicListType() : Type(Type::Kind::VariadicList) {}
        ~VariadicListType() override = default;

        [[nodiscard]] bool matches(const Type *other) const override {
            if (auto otherVariadicList = dynamic_cast<const VariadicListType*>(other)) {
                return true;
            }
            return false;
        }
    };
}

#endif //OXYIR_VARIADICLISTTYPE_H