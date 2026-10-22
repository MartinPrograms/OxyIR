#ifndef OXYIR_VOIDTYPE_H
#define OXYIR_VOIDTYPE_H

#include "Type.h"

namespace Oxy {
    class VoidType : public Type {
    public:
        VoidType() : Type(Kind::Void) {}
        ~VoidType() override = default;

        [[nodiscard]] bool matches(const Type *other) const override {
            if (auto otherVoid = dynamic_cast<const VoidType*>(other)) {
                return true;
            }

            return false;
        }
    };
} // Oxy

#endif //OXYIR_VOIDTYPE_H