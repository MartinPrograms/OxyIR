#ifndef OXYIR_EQUALITYOPERATION_H
#define OXYIR_EQUALITYOPERATION_H

#include "../Instruction.h"

// eq, ne, lt, gt, le, ge
// lt, gr, le, ge have signs

namespace Oxy::Instructions {
    class EqualityOperation : public Instruction {
    public:
        enum class Opcode {
            Eq,
            Ne,
            Lt,
            Gt,
            Le,
            Ge
        };

        enum class Signedness { Signed, Unsigned, None };

        EqualityOperation(Opcode opcode, Signedness signedness, Value* lhs, Value* rhs) : _opcode(opcode),
            _signedness(signedness), _lhs(lhs), _rhs(rhs) {}

        ~EqualityOperation() override = default;

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

#endif //OXYIR_EQUALITYOPERATION_H