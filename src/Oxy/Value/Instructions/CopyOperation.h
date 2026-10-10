#ifndef OXYIR_COPYOPERATION_H
#define OXYIR_COPYOPERATION_H

// | `copy`      | `type operand`                                                | `type`      | Copies a value from one variable to another.                                                                                                                                                                                                                                                                                                                          |

#include "../Instruction.h"

namespace Oxy::Instructions {
    class CopyOperation : public Instruction {
    public:
        explicit CopyOperation(Value* operand) : _operand(operand) {}
        ~CopyOperation() override = default;

        [[nodiscard]] Value* getOperand() const {
            return _operand;
        }

        [[nodiscard]] Type* getType() const override {
            return _operand->getType();
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Value* _operand;
    };
} // Instructions

#endif //OXYIR_COPYOPERATION_H