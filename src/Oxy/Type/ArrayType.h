#ifndef OXYIR_ARRAYTYPE_H
#define OXYIR_ARRAYTYPE_H

#include "Type.h"
#include <memory>

namespace Oxy {
    class ArrayType : public Type {
    public:
        explicit ArrayType(Type* elementType, size_t size, bool isPacked = false, size_t alignment = 0);
        ~ArrayType() override;

        [[nodiscard]] Type* getElementType() const;
        [[nodiscard]] size_t getSize() const;
        [[nodiscard]] bool isPacked() const;
        [[nodiscard]] size_t getAlignment() const;

    private:
        Type* _elementType;
        size_t _size;
        bool _isPacked;
        size_t _alignment;
    };
} // Oxy

#endif //OXYIR_ARRAYTYPE_H