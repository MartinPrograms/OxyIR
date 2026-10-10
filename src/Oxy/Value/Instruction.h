#ifndef OXYIR_INSTRUCTION_H
#define OXYIR_INSTRUCTION_H

#include "Value.h"
#include "Oxy/InstructionVisitor.h"

// Base class for all instructions in the IR, every instruction inherits from this class.

namespace Oxy {
    class Instruction : public Value {
    public:
        ~Instruction() override = default;
        virtual void accept(InstructionVisitor* visitor) = 0;
    };
}

#endif //OXYIR_INSTRUCTION_H