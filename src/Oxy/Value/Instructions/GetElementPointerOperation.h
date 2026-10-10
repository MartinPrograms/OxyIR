#ifndef OXYIR_GETELEMENTPOINTEROPERATION_H
#define OXYIR_GETELEMENTPOINTEROPERATION_H

// | `gep`          | `type operand, index1, index2, ...` | `ptr`       | Computes the address of a sub-element of an aggregate data structure (struct or array). The first operand must be a pointer to the aggregate type, and the indices specify the path to the sub-element. Index integers are the same size as the system's pointer size.                                     |

#include "../Instruction.h"
#include <vector>

namespace Oxy::Instructions {
    class GetElementPointerOperation : public Instruction {
    public:
        GetElementPointerOperation(Type* type, Value* operand, std::vector<Value*> indices)
            : _type(type), _operand(operand), _indices(std::move(indices)) {}
        ~GetElementPointerOperation() override = default;

        [[nodiscard]] Value* getOperand() const {
            return _operand;
        }

        [[nodiscard]] const std::vector<Value*>& getIndices() const {
            return _indices;
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
        std::vector<Value*> _indices;
    };
} // Instructions

#endif //OXYIR_GETELEMENTPOINTEROPERATION_H