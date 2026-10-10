#ifndef OXYIR_POINTERTYPE_H
#define OXYIR_POINTERTYPE_H

#include "Type.h"

// A pointer does not have information about what it points to, it is just a pointer.

namespace Oxy {
    class PointerType : public Type {
    public:
        PointerType() : Type(Kind::Pointer) {}
        ~PointerType() override = default;
    }; // Oxy
}

#endif //OXYIR_POINTERTYPE_H