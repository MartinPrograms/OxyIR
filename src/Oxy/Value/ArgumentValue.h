#ifndef OXYIR_ARGUMENTVALUE_H
#define OXYIR_ARGUMENTVALUE_H

#include "Value.h"

namespace Oxy {
    class ArgumentValue : public Value {
    public:
        ArgumentValue(Type* type, int index) : _type(type), _arg(index) {}
        ~ArgumentValue() override = default;

        [[nodiscard]] Type* getType() const override {
            return _type;
        }

        [[nodiscard]] int getIndex() const {
            return _arg;
        }

    private:
        Type* _type;
        int _arg;
    };
} // Oxy

#endif //OXYIR_ARGUMENTVALUE_H