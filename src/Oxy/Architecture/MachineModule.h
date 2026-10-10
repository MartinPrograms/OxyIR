#ifndef OXYIR_MACHINEMODULE_H
#define OXYIR_MACHINEMODULE_H

#include "../Module.h"

namespace Oxy::Architecture {
    struct TargetSpecificData {
        virtual ~TargetSpecificData() = default;
    };

    typedef struct MachineBlock MachineBlock;

    struct StackSlot {
        uint32_t base; // base register for the stack slot, typically RBP or RSP
        int64_t offset; // offset from the base register to the stack slot
    };

    struct MachineOperand {
        enum class Kind { VirtualRegister, PhysicalRegister, Immediate, TargetSpecific, FloatValue, StackSlot, Block };

        Kind kind;
        uint8_t flags;
        std::variant<
            uint32_t, // for virtual and physical registers, the value is the register number
            int64_t, // for immediate values
            double, // for float values
            MachineBlock*, // for block operands
            StackSlot, // for stack slot operands
            TargetSpecificData* // for target specific operands
        > value;
    };

    struct MachineInstruction {
        uint32_t opcode; // target specific opcode
        uint32_t flags; // target specific flags
        std::vector<MachineOperand> operands;
    };

    struct MachineBlock {
        BasicBlock* id;
        std::vector<MachineInstruction> instructions;
        std::vector<BasicBlock*> successors;
        std::vector<BasicBlock*> predecessors;
    };

    struct MachineFunction {
        std::string name;
        std::vector<std::pair<BasicBlock*, MachineBlock>> blocks;
        uint32_t vregInfo;
        int64_t stackBase; // base offset for the function's stack frame, used for allocating stack slots
        uint32_t nextVirtualRegisterId = 0;
    };

    class RegisterAllocator {
    public:
        virtual ~RegisterAllocator() = default;
        virtual void allocate(MachineFunction* machine_func) = 0;
    };

    // The MachineModule is an interface inherited by architecture specific modules that will be used for code gen.
    class MachineModule : public InstructionVisitor {
    public:
        MachineModule(Module *module, std::unique_ptr<RegisterAllocator> register_allocator) : _module(module), _context(module->getContext()), _register_allocator(std::move(register_allocator)) {};
        ~MachineModule() override = default;

        virtual void lower() = 0;

        [[nodiscard]] const std::vector<std::unique_ptr<MachineFunction>>& getFunctions() const {
            return _functions;
        }

    protected:
        Module *_module;
        Context *_context;
        std::unique_ptr<RegisterAllocator> _register_allocator;

        std::vector<std::unique_ptr<MachineFunction>> _functions;
    };
}

#endif //OXYIR_MACHINEMODULE_H
