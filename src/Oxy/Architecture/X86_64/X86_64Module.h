#ifndef OXYIR_X86_64MODULE_H
#define OXYIR_X86_64MODULE_H

#include <unordered_set>

#include "SimpleRegisterAllocator.h"
#include "X86_64.h"
#include "../MachineModule.h"

namespace Oxy::Architecture {
    class X86_64Module final : public MachineModule {
    public:
        struct MemoryOperand {
            uint64_t base;
            uint64_t index;
            uint8_t scale;
            int64_t displacement;
        };

        explicit X86_64Module(Module *module, std::unique_ptr<RegisterAllocator> allocator = nullptr)
            : MachineModule(module, std::move(allocator)) {
            if (!allocator) {
                _register_allocator = std::make_unique<X86_64::SimpleRegisterAllocator>();
            }
        }

        void lower() override;
    private:
        std::vector<MemoryOperand> _memoryOperands;
        std::vector<std::unordered_map<Value*, uint32_t>> _vregTables = {{}}; // one per function, indexed by virtual register number
        MachineFunction *_currentFunction = nullptr;
        Function * _currentIRFunction = nullptr;
        MachineBlock *_currentBlock = nullptr;
        std::unordered_map<Instructions::AllocaOperation*, int64_t> _allocaOffsets;
        std::unordered_map<Instruction*, StackSlot> _stackSlots;

        size_t getBlockIndex(Function * func, BasicBlock * get);
        void lower_function(Function *func, MachineFunction *machine_func);
        void lower_block(BasicBlock *block, MachineBlock *machine_block);
        void lower_instruction(Instruction *instr);

        uint32_t getVirtualRegister() {
            return _currentFunction->nextVirtualRegisterId++; // Start virtual registers at 500 to avoid conflicts with physical registers
        }

        std::unordered_map<Value*, uint32_t>& getCurrentVregTable() {
            return _vregTables.back();
        }

        MachineOperand resolveValue(Value * value);

        int64_t getTypeSize(Type *get_type);
        void visit(Instructions::AllocaOperation *instr) override;
        void visit(Instructions::AsmOperation *instr) override;
        void visit(Instructions::BinaryOperation *instr) override;
        void visit(Instructions::BitCastOperation *instr) override;
        void visit(Instructions::CallOperation *instr) override;
        void visit(Instructions::CastOperation *instr) override;
        void visit(Instructions::CopyOperation *instr) override;
        void visit(Instructions::CustomInstruction *instr) override;
        void visit(Instructions::EqualityOperation *instr) override;
        void visit(Instructions::GetElementPointerOperation *instr) override;
        void visit(Instructions::JumpNotZeroOperation *instr) override;
        void visit(Instructions::JumpOperation *instr) override;
        void visit(Instructions::LoadOperation *instr) override;
        void visit(Instructions::PhiOperation *instr) override;
        void visit(Instructions::ReturnOperation *instr) override;
        void visit(Instructions::StoreOperation *instr) override;
        void visit(Instructions::UnaryOperation *instr) override;
        void visit(Instructions::VaArgOperation *instr) override;
        void visit(Instructions::VaCopyOperation *instr) override;
        void visit(Instructions::VaStartOperation *instr) override;
    };
}


#endif //OXYIR_X86_64MODULE_H
