#ifndef OXYIR_VALUE_H
#define OXYIR_VALUE_H

#include <memory>
#include <string>

// Base class for all values in the IR. This includes functions, globals, constants and instructions.

#include "../Type/Type.h"

namespace Oxy {
    class Value {
    public:
        virtual ~Value() = default;
        [[nodiscard]] virtual Type* getType() const = 0;
    };
}

#endif //OXYIR_VALUE_H