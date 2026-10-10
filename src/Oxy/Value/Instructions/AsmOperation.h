#ifndef OXYIR_ASMOPERATION_H
#define OXYIR_ASMOPERATION_H

// | `asm`       | `type output, "assembly code %0, %1", type1 arg1, type2 arg2` | `type`      | Embeds inline assembly code. The output operand specifies the type and variable that will hold the result of the assembly code. The assembly code is a string that can include placeholders for the arguments, which are specified after the assembly code. The result type must match the output operand type.                                                       |

#include "../Instruction.h"
#include <vector>

namespace Oxy::Instructions {
    class AsmOperation : public Instruction {
    public:
        struct Operand {
            Type* type;
            Value* value;
        };

        AsmOperation(Type* outputType, std::string assemblyCode, std::vector<Operand> args)
            : _outputType(outputType), _assemblyCode(std::move(assemblyCode)), _args(std::move(args)) {}

        ~AsmOperation() override = default;

        [[nodiscard]] Type* getType() const override {
            return _outputType;
        }

        [[nodiscard]] const std::string& getAssemblyCode() const {
            return _assemblyCode;
        }

        [[nodiscard]] const std::vector<Operand>& getArgs() const {
            return _args;
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        Type* _outputType;
        std::string _assemblyCode;
        std::vector<Operand> _args;
    };
} // Instructions

#endif //OXYIR_ASMOPERATION_H