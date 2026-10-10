#ifndef OXYIR_CUSTOMINSTRUCTIONSET_H
#define OXYIR_CUSTOMINSTRUCTIONSET_H

#include "./Value/Instruction.h"
#include <vector>
#include <map>
#include <functional>

#include "Value/Instructions/CustomInstruction.h"

namespace Oxy {
    class CustomInstructionSet {
    public:
        explicit CustomInstructionSet(std::string name) : _name(std::move(name)) {};
        ~CustomInstructionSet() = default;

        void addInstruction(std::string name, std::function<std::unique_ptr<Instructions::CustomInstruction>(Type* returnType, std::vector<Value*> values)> creator) {
            _instructions[name] = std::move(creator);
        }

        [[nodiscard]] const std::string& getName() const {
            return _name;
        }

        [[nodiscard]] const std::map<std::string, std::function<std::unique_ptr<Instructions::CustomInstruction>(Type* returnType, std::vector<Value*> values)>>& getInstructions() const {
            return _instructions;
        }

        [[nodiscard]] bool hasInstruction(const std::string& name) const {
            return _instructions.find(name) != _instructions.end();
        }

        [[nodiscard]] std::function<std::unique_ptr<Instructions::CustomInstruction>(Type* returnType, std::vector<Value*> values)> getInstruction(const std::string& name) const {
            auto it = _instructions.find(name);
            if (it == _instructions.end()) {
                throw std::runtime_error("Custom instruction not found: " + name);
            }
            return it->second;
        }

    private:
        std::string _name;
        std::map<std::string, std::function<std::unique_ptr<Instructions::CustomInstruction>(Type* returnType, std::vector<Value*> values)>> _instructions;
    };
}

#endif //OXYIR_CUSTOMINSTRUCTIONSET_H