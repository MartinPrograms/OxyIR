#ifndef OXYIR_BINARYOPERATION_H
#define OXYIR_BINARYOPERATION_H

// add, sub, mul, div, rem, and, or, xor

#include "../Instruction.h"


namespace Oxy::Instructions {
    class BinaryOperation : public Instruction {
    public:
        enum class Signedness { Signed, Unsigned, None };

        enum class Opcode {
            Add,
            Sub,
            Mul,
            Div,
            Rem,
            And,
            Or,
            Xor,
            Shl,
            Shr
        };

        BinaryOperation(Opcode opcode, Signedness signedness, Value* lhs, Value* rhs) : _opcode(opcode),
            _signedness(signedness), _lhs(lhs), _rhs(rhs) {}

        ~BinaryOperation() override = default;

        [[nodiscard]] Opcode getOpcode() const {
            return _opcode;
        }

        [[nodiscard]] Signedness getSignedness() const {
            return _signedness;
        }

        [[nodiscard]] Value* getLHS() const {
            return _lhs;
        }

        [[nodiscard]] Value* getRHS() const {
            return _rhs;
        }

        [[nodiscard]] Type* getType() const override {
            // This is type checked later during a type check pass.
            return _lhs->getType();
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Opcode _opcode;
        Signedness _signedness;
        Value* _lhs;
        Value* _rhs;
    };
} // Instructions
// Oxy

#endif //OXYIR_BINARYOPERATION_H