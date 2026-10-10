#ifndef OXYIR_STRUCTTYPE_H
#define OXYIR_STRUCTTYPE_H

#include "Type.h"
#include <vector>
#include <memory>

namespace Oxy {
    class StructType : public Type {
    public:
        explicit StructType(std::vector<Type*> memberTypes, size_t alignment = 0);
        ~StructType() override = default;

        [[nodiscard]] const std::vector<Type*>& getMemberTypes() const;
        [[nodiscard]] size_t getAlignment() const;

    private:
        std::vector<Type*> _memberTypes;
        size_t _alignment;
    };
} // Oxy

#endif //OXYIR_STRUCTTYPE_H