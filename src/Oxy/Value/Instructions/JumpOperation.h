#ifndef OXYIR_JUMPOPERATION_H
#define OXYIR_JUMPOPERATION_H

#include "../Instruction.h"

// jmp basic_block

namespace Oxy {
    class BasicBlock;
}

namespace Oxy::Instructions {
    class JumpOperation : public Instruction {
    public:
        explicit JumpOperation(const BasicBlock *target) : _target(target) {}
        ~JumpOperation() override = default;

        [[nodiscard]] const BasicBlock* getTarget() const {
            return _target;
        }

        [[nodiscard]] Type* getType() const override {
            return nullptr; // Jump operations do not produce a value
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        const BasicBlock* _target;
    };
} // Instructions

#endif //OXYIR_JUMPOPERATION_H