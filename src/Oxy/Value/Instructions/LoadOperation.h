#ifndef OXYIR_LOADOPERATION_H
#define OXYIR_LOADOPERATION_H

// | `load`         | `type operand`                      | `type`      | Loads a value from memory. The operand must be a pointer to the type of the result.                                                                                                                                                                                                                        |

#include "../Instruction.h"

namespace Oxy::Instructions {
    class LoadOperation : public Instruction {
    public:
        explicit LoadOperation(Type* type, Value* operand) : _type(type), _operand(operand) {}
        ~LoadOperation() override = default;

        [[nodiscard]] Value* getOperand() const {
            return _operand;
        }

        [[nodiscard]] Type* getType() const override {
            return _type;
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Type* _type;
        Value* _operand;
    };
}

#endif //OXYIR_LOADOPERATION_H