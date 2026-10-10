#ifndef OXYIR_CUSTOMINSTRUCTION_H
#define OXYIR_CUSTOMINSTRUCTION_H

#include "../Instruction.h"
#include <vector>

namespace Oxy::Instructions {
    class CustomInstruction : public Instruction {
    public:
        CustomInstruction(std::string name, std::vector<Value*> operands, Type* opType, bool producesValue)
            : _name(std::move(name)), _operands(std::move(operands)), _resultType(opType), _producesValue(producesValue) {}

        ~CustomInstruction() override = default;

        [[nodiscard]] const std::string& getName() const {
            return _name;
        }

        [[nodiscard]] const std::vector<Value*>& getOperands() const {
            return _operands;
        }

        [[nodiscard]] Type* getType() const override {
            return _resultType;
        }

        [[nodiscard]] bool producesValue() const {
            return _producesValue;
        }

        void accept(InstructionVisitor* visitor) override {
            visitor->visit(this);
        }

    private:
        std::string _name;
        std::vector<Value*> _operands;
        Type* _resultType;
        bool _producesValue;
    };
} // Instructions

#endif //OXYIR_CUSTOMINSTRUCTION_H