#ifndef OXYIR_CALLOPERATION_H
#define OXYIR_CALLOPERATION_H

#include "../Instruction.h"
#include "../../Type/FunctionType.h"
#include <vector>

// | `call`      | `type operand, type1 arg1, type2 arg2`                        | `type`      | Calls a function. The first operand must be a pointer to a function type, and the arguments must match the parameter types of the function. The result type must match the return type of the function. The variadic arguments, and normal arguments must be seperated with `...` as shown in the operands. If the function is `void` `%result =` needs to be omitted |

namespace Oxy::Instructions {
    class CallOperation : public Instruction {
    public:
        CallOperation(Value* callee, FunctionType* functionType, std::vector<Value*> args) : _callee(callee), _functionType(functionType), _args(std::move(args)) {}
        ~CallOperation() override = default;

        [[nodiscard]] Value* getCallee() const {
            return _callee;
        }

        [[nodiscard]] FunctionType* getFunctionType() const {
            return _functionType;
        }

        [[nodiscard]] const std::vector<Value*>& getArgs() const {
            return _args;
        }

        [[nodiscard]] Type* getType() const override {
            return _functionType->getReturnType();
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Value* _callee;
        FunctionType* _functionType;
        std::vector<Value*> _args;
    };
} // Instructions

#endif //OXYIR_CALLOPERATION_H