#ifndef OXYIR_VASTART_H
#define OXYIR_VASTART_H

// | `vastart`   | `operand`                                                     | `void`      | Initializes a `va_list` variable for use in a variadic function. The operand must be a pointer to a `va_list` type, and no type is required since the operand is always of `va_list`.                                                                                                                                                                                 |

#include "../Instruction.h"

namespace Oxy::Instructions {
    class VaStartOperation : public Instruction {
    public:
        explicit VaStartOperation(Value* vaListOperand) : _vaListOperand(vaListOperand) {}
        ~VaStartOperation() override = default;

        [[nodiscard]] Value* getVaListOperand() const {
            return _vaListOperand;
        }

        [[nodiscard]] Type* getType() const override {
            return nullptr; // VaStart does not produce a value
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Value* _vaListOperand;
    };
} // Instructions

#endif //OXYIR_VASTART_H