#ifndef OXYIR_TYPE_H
#define OXYIR_TYPE_H

namespace Oxy {
    class Type {
    public:
        enum class Kind { Void, Primitive, Pointer, Array, Struct, Function, VariadicList };

        explicit Type(Kind kind) : _kind(kind) {}
        virtual ~Type() = default;

        [[nodiscard]] Kind getKind() const { return _kind; }

        [[nodiscard]] virtual bool matches(const Type *other) const {
            return _kind == other->_kind;
        }

    private:
        Kind _kind;
    };
}

#endif