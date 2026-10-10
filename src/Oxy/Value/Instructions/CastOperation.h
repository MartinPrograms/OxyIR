#ifndef OXYIR_CASTOPERATION_H
#define OXYIR_CASTOPERATION_H

/*
| Instruction  | Operands                          | Result Type   | Description                                                                                                                                               |
|--------------|-----------------------------------|---------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| `trunc`      | `type operand`                    | `type`        | Truncates a value to a smaller type. The operand must be an integer type, and the result type must be a smaller integer.                                  |
| `zext`       | `type operand`                    | `type`        | Zero-extends a value to a larger type. The operand must be an integer type, and the result type must be a larger integer type.                            |
| `sext`       | `type operand`                    | `type`        | Sign-extends a value to a larger type. The operand must be an integer type, and the result type must be a larger integer type.                            |
| `fptrunc`    | `type operand`                    | `type`        | Truncates a floating-point value to a smaller type. The operand must be a floating-point type, and the result type must be a smaller floating-point type. |
| `fpext`      | `type operand`                    | `type`        | Extends a floating-point value to a larger type. The operand must be a floating-point type, and the result type must be a larger floating-point type.     |
| `ptrtoint`   | `type operand`                    | `type`        | Converts a pointer to an integer. The operand must be a pointer type, and the result type must be an integer type.                                        |
| `inttoptr`   | `type operand`                    | `type`        | Converts an integer to a pointer. The operand must be an integer type, and the result type must be a pointer type.                                        |
| `inttofloat` | `result_type source_type operand` | `result_type` | Converts an integer to a floating point number.                                                                                                           |
| `floattoint` | `result_type source_type operand` | `result_type` | Converts a floating point number to an integer.                                                                                                           |

The following instructions have signed and unsigned version:
- `trunc`
- `zext`
- `sext`
- `inttofloat` (if the source_type's sign should be taken into account)

But `ptrtoint` and `inttoptr` do not have signed or unsigned versions, as they operate on pointers and integers, not values.
`floattoint` always results in a signed integer.

*/

#include "../Instruction.h"

namespace Oxy::Instructions {
    class CastOperation : public Instruction {
    public:
        enum class Opcode {
            Trunc,
            ZExt,
            SExt,
            FpTrunc,
            FpExt,
            PtrToInt,
            IntToPtr,
            IntToFloat,
            FloatToInt
        };

        enum class Signedness { Signed, Unsigned, None };

        CastOperation(Opcode opcode, Signedness signedness, Value* operand, Type* resultType)
            : _opcode(opcode), _signedness(signedness), _operand(operand), _resultType(resultType) {}

        ~CastOperation() override = default;

        [[nodiscard]] Opcode getOpcode() const {
            return _opcode;
        }

        [[nodiscard]] Signedness getSignedness() const {
            return _signedness;
        }

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
        Opcode _opcode;
        Signedness _signedness;
        Value* _operand;
        Type* _resultType;
    };
} // Instructions

#endif //OXYIR_CASTOPERATION_H