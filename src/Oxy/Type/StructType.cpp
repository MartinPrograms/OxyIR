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

    bool StructType::matches(const Type *other) const {
        StructType* otherStruct = dynamic_cast<StructType*>(const_cast<Type*>(other));

        if (!otherStruct)
            return false;

        if (_memberTypes.size() != otherStruct->_memberTypes.size()) {
            return false;
        }

        for (size_t i = 0; i < _memberTypes.size(); ++i) {
            if (i >= otherStruct->_memberTypes.size() || !_memberTypes[i]->matches(otherStruct->_memberTypes[i])) {
                return false;
            }
        }

        if (_alignment != otherStruct->_alignment) {
            return false;
        }

        return true;
    }
} // Oxy