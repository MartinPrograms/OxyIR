#ifndef OXYIR_TYPEVALUE_H
#define OXYIR_TYPEVALUE_H

#include "Value.h"

namespace Oxy {
    class TypeValue: public Value {
    public:
        explicit TypeValue(Type* type) : _type(type) {}
        ~TypeValue() override = default;

        [[nodiscard]] Type* getType() const override {
            return _type;
        }

    private:
        Type* _type;
    };
}

#endif //OXYIR_TYPEVALUE_H
