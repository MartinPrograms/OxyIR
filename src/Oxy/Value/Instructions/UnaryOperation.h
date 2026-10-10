#ifndef OXYIR_UNARYOPERATION_H
#define OXYIR_UNARYOPERATION_H

// neg, not

#include "../Instruction.h"

namespace Oxy::Instructions {
    class UnaryOperation : public Instruction {
    public:
        enum class Opcode {
            Neg,
            Not
        };

        UnaryOperation(Opcode opcode, Value* operand) : _opcode(opcode), _operand(operand) {}

        ~UnaryOperation() override = default;

        [[nodiscard]] Opcode getOpcode() const {
            return _opcode;
        }

        [[nodiscard]] Value* getOperand() const {
            return _operand;
        }

        [[nodiscard]] Type* getType() const override {
            // This is type checked later during a type check pass.
            return _operand->getType();
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Opcode _opcode;
        Value* _operand;
    };
} // Instructions

#endif //OXYIR_UNARYOPERATION_H