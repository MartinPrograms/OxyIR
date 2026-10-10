#include "ArrayType.h"

namespace Oxy {
    ArrayType::ArrayType(Type* elementType, size_t size, bool isPacked, size_t alignment) : Type(Kind::Array) {
        _elementType = elementType;
        _size = size;
        _isPacked = isPacked;
        _alignment = alignment;
    }

    ArrayType::~ArrayType() = default;

    Type* ArrayType::getElementType() const {
        return _elementType;
    }

    size_t ArrayType::getSize() const {
        return _size;
    }

    bool ArrayType::isPacked() const {
        return _isPacked;
    }

    size_t ArrayType::getAlignment() const {
        return _alignment;
    }
} // Oxy