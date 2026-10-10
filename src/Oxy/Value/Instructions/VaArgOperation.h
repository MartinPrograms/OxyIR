#ifndef OXYIR_VAARGOPERATION_H
#define OXYIR_VAARGOPERATION_H

#include "../Instruction.h"

// | `vaarg`     | `result_type va_list_operand`                                 | `type`      | Retrieves the next argument from a variadic function. The operand must be a pointer to a `va_list` type, and the result type must be the type of the argument being retrieved.                                                                                                                                                                                        |

namespace Oxy::Instructions {
    class VaArgOperation : public Instruction {
    public:
        VaArgOperation(Type* resultType, Value* vaListOperand) : _resultType(resultType), _vaListOperand(vaListOperand) {}
        ~VaArgOperation() override = default;

        [[nodiscard]] Value* getVaListOperand() const {
            return _vaListOperand;
        }

        [[nodiscard]] Type* getType() const override {
            return _resultType;
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Type* _resultType;
        Value* _vaListOperand;
    };
} // Instructions

#endif //OXYIR_VAARGOPERATION_H