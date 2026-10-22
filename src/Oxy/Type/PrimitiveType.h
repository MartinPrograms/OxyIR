#ifndef OXYIR_PRIMITIVETYPE_H
#define OXYIR_PRIMITIVETYPE_H

#include "Type.h"
#include "Type.h"

namespace Oxy {
    class PrimitiveType : public Type {
    public:
        enum class Kind {
            I1,
            I8,
            I16,
            I32,
            I64,
            F32,
            F64
        };

        explicit PrimitiveType(Kind kind) : Type(Type::Kind::Primitive), _primitiveKind(kind) {}

        [[nodiscard]] Kind getPrimitiveKind() const {
            return _primitiveKind;
        }

        ~PrimitiveType() override = default;

        [[nodiscard]]bool matches(const Type *other) const override {
            if (auto otherPrimitive = dynamic_cast<const PrimitiveType*>(other)) {
                return _primitiveKind == otherPrimitive->_primitiveKind;
            }

            return false;
        }

    private:
        Kind _primitiveKind;
    };
} // Oxy

#endif //OXYIR_PRIMITIVETYPE_H