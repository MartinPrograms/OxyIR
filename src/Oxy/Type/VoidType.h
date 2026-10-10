#ifndef OXYIR_VOIDTYPE_H
#define OXYIR_VOIDTYPE_H

#include "Type.h"

namespace Oxy {
    class VoidType : public Type {
    public:
        VoidType() : Type(Kind::Void) {}
        ~VoidType() override = default;
    };
} // Oxy

#endif //OXYIR_VOIDTYPE_H