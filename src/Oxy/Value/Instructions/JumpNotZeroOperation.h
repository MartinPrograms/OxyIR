#ifndef OXYIR_JUMPNOTZEROOPERATION_H
#define OXYIR_JUMPNOTZEROOPERATION_H

#include "../Instruction.h"

// jnz value, true_block, false_block

namespace Oxy {
    class BasicBlock;
}

namespace Oxy::Instructions {
    class JumpNotZeroOperation : public Instruction {
    public:
        JumpNotZeroOperation(Value* condition, const BasicBlock* trueTarget, const BasicBlock* falseTarget)
            : _condition(condition), _trueTarget(trueTarget), _falseTarget(falseTarget) {}
        ~JumpNotZeroOperation() override = default;

        [[nodiscard]] Value* getCondition() const {
            return _condition;
        }

        [[nodiscard]] const BasicBlock* getTrueTarget() const {
            return _trueTarget;
        }

        [[nodiscard]] const BasicBlock* getFalseTarget() const {
            return _falseTarget;
        }

        [[nodiscard]] Type* getType() const override {
            return nullptr; // Jump operations do not produce a value
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Value* _condition;
        const BasicBlock* _trueTarget;
        const BasicBlock* _falseTarget;
    };
} // Instructions

#endif //OXYIR_JUMPNOTZEROOPERATION_H