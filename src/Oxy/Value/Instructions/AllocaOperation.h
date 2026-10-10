#ifndef OXYIR_ALLOCAOPERATION_H
#define OXYIR_ALLOCAOPERATION_H

#include "../Instruction.h"

// | `alloca`       | `type [, count]`                    | `ptr`       | Allocates T * N bytes of memory on the stack, where T is the type of the result and N is the count, count can be optional and defaults to 1. Count must be a constant expression that evaluates to an integer.                                                                                             |
// | `alloca_align` | `type, alignment [, count]`         | `ptr`       | Allocates T * N bytes of memory on the stack, where T is the type of the result and N is the count, count can be optional and defaults to 1. Alignment must be a constant expression that evaluates to a power of 2 integer. Alignment must be greater than or equal to the natural alignment of the type. |

namespace Oxy::Instructions {
    class AllocaOperation : public Instruction {
    public:
        AllocaOperation(Type* type, Value* count = nullptr, size_t alignment = 0) : _type(type), _count(count), _alignment(alignment) {}
        ~AllocaOperation() override = default;

        [[nodiscard]] Type* getType() const override {
            return _type;
        }

        [[nodiscard]] Value* getCount() const {
            return _count;
        }

        [[nodiscard]] size_t getAlignment() const {
            return _alignment;
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Type* _type;
        Value* _count; // Can be nullptr, which means a count of 1
        size_t _alignment; // 0 means natural alignment
    };
} // Instructions

#endif //OXYIR_ALLOCAOPERATION_H