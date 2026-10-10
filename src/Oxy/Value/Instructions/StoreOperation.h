#ifndef OXYIR_STOREOPERATION_H
#define OXYIR_STOREOPERATION_H

#include "../Instruction.h"

namespace Oxy::Instructions {
    class StoreOperation : public Instruction {
    public:
        StoreOperation(Value* destination, Value* value) : _destination(destination), _value(value) {}
        ~StoreOperation() override = default;

        [[nodiscard]] Value* getDestination() const {
            return _destination;
        }

        [[nodiscard]] Value* getValue() const {
            return _value;
        }

        [[nodiscard]] Type* getType() const override {
            // Store doesn't produce a value, so we can return nullptr or void type.
            return nullptr;
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Value* _destination;
        Value* _value;
    };
} // Instructions

#endif //OXYIR_STOREOPERATION_H