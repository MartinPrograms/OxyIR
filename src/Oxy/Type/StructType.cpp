#include "StructType.h"

namespace Oxy {
    StructType::StructType(std::vector<Type*> memberTypes, size_t alignment) : Type(Kind::Struct) {
        _memberTypes = std::move(memberTypes);
        _alignment = alignment;
    }

    const std::vector<Type*> & StructType::getMemberTypes() const {
        return _memberTypes;
    }

    size_t StructType::getAlignment() const {
        return _alignment;
    }
} // Oxy