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

        explicit PrimitiveType(Kind kind) : Type(Type::Kind::Primitive), _kind(kind) {}

        [[nodiscard]] Kind getPrimitiveKind() const {
            return _kind;
        }

        ~PrimitiveType() override = default;

    private:
        Kind _kind;
    };
} // Oxy

#endif //OXYIR_PRIMITIVETYPE_H