#include <Oxy/Module.h>

#include "Oxy/IRParser.h"
#include "test.oxyir"

class VolatileLoad : public Oxy::Instructions::CustomInstruction {
public:
    // XXX.load <type> <pointer>
    // Where XXX is the instruction set
    explicit VolatileLoad(std::string identifier, Oxy::Type* type, Value* pointer) : CustomInstruction(std::move(identifier), {pointer}, type, true), _pointer(pointer) {}

    [[nodiscard]] Value* getPointer() const {
        return _pointer;
    }

    static constexpr const char* Name = "load";

private:
    Value* _pointer;
};


int main() {
    Oxy::Context context;

    auto volatileSet = context.createCustomInstructionSet("volatile");
    volatileSet->addInstruction(VolatileLoad::Name, [&](Oxy::Type* type, std::vector<Oxy::Value*> operands) -> std::unique_ptr<Oxy::Instructions::CustomInstruction> {
        if (operands.size() != 1) {
            throw std::runtime_error("Volatile load instruction requires exactly one operand");
        }
        return std::make_unique<VolatileLoad>(volatileSet->getName() + "." + VolatileLoad::Name, type, operands[0]);
    });

    Oxy::IRParser parser(&context, testingCode, Oxy::Module::Architecture::Amd64, "parser_test");
    auto module = parser.parse();

    return 0;
}
