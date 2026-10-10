#ifndef OXYIR_BITCASTOPERATION_H
#define OXYIR_BITCASTOPERATION_H

// | `cast`      | `type operand`                                                | `type`      | Casts a value from one type to another. Casts memory verbatim. Size of source & destination type must be equal.                                                                                                                                                                                                                                                       |

#include "../Instruction.h"

namespace Oxy::Instructions {
    class BitCastOperation : public Instruction {
    public:
        BitCastOperation(Value* operand, Type* resultType) : _operand(operand), _resultType(resultType) {}
        ~BitCastOperation() override = default;

        [[nodiscard]] Value* getOperand() const {
            return _operand;
        }

        [[nodiscard]] Type* getType() const override {
            return _resultType;
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Value* _operand;
        Type* _resultType;
    };
} // Instructions

#endif //OXYIR_BITCASTOPERATION_H