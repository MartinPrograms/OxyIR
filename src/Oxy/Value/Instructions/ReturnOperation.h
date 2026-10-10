#ifndef OXYIR_RETURN_H
#define OXYIR_RETURN_H

#include "../Instruction.h"
#include <memory>

namespace Oxy::Instructions {
    class ReturnOperation : public Instruction {
    public:
        // To return void just pass nullptr as the return value
        explicit ReturnOperation(Value* returnValue = nullptr)
            : _returnValue(returnValue) {}

        ~ReturnOperation() override = default;

        [[nodiscard]] Value* getReturnValue() const {
            return _returnValue;
        }

        [[nodiscard]] Type* getType() const override {
            return getReturnValue() ? getReturnValue()->getType() : nullptr;
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Value* _returnValue;
    };
}

#endif //OXYIR_RETURN_H
