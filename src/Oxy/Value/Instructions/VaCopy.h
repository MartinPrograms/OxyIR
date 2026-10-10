#ifndef OXYIR_VACOPY_H
#define OXYIR_VACOPY_H

// | `vacopy`    | `dest_va_list_operand, src_va_list_operand`                   | `void`      | Copies the state of one `va_list` variable to another. Both operands must be pointers to `va_list` types. This is useful for saving the state of a `va_list` before retrieving arguments, and restoring it later if needed.                                                                                                                                           |

#include "../Instruction.h"

namespace Oxy::Instructions {
    class VaCopyOperation : public Instruction {
    public:
        VaCopyOperation(Value* destVaListOperand, Value* srcVaListOperand) : _destVaListOperand(destVaListOperand), _srcVaListOperand(srcVaListOperand) {}
        ~VaCopyOperation() override = default;

        [[nodiscard]] Value* getDestVaListOperand() const {
            return _destVaListOperand;
        }

        [[nodiscard]] Value* getSrcVaListOperand() const {
            return _srcVaListOperand;
        }

        [[nodiscard]] Type* getType() const override {
            return nullptr; // VaCopy does not produce a value
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Value* _destVaListOperand;
        Value* _srcVaListOperand;
    };
} // Instructions

#endif //OXYIR_VACOPY_H