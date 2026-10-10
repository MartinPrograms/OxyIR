#ifndef OXYIR_PHIOPERATION_H
#define OXYIR_PHIOPERATION_H

#include "../Instruction.h"
#include <vector>

// | `phi`       | `type operand1, @label1, operand2, @label2, ...`              | `type`      | Selects a value based on the control flow path taken to reach the current block. The operands are pairs of values and labels, where each value is associated with a label that represents a control flow path. The result type must be the same as the type of the values. Must be the first instruction(s) in a basic block.                                         |

namespace Oxy {
    class BasicBlock;
}

namespace Oxy::Instructions {
    class PhiOperation : public Instruction {
    public:
        struct Incoming {
            Value* value;
            BasicBlock* block;
        };

        PhiOperation(Type* type, std::vector<Incoming> incoming) : _type(type), _incoming(std::move(incoming)) {}
        ~PhiOperation() override = default;

        [[nodiscard]] const std::vector<Incoming>& getIncoming() const {
            return _incoming;
        }

        [[nodiscard]] Type* getType() const override {
            return _type;
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Type* _type;
        std::vector<Incoming> _incoming;
    };
} // Instructions

#endif //OXYIR_PHIOPERATION_H