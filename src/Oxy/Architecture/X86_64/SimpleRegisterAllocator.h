#ifndef OXYIR_SIMPLEREGISTERALLOCATOR_H
#define OXYIR_SIMPLEREGISTERALLOCATOR_H

#include "X86_64.h"
#include "../MachineModule.h"

namespace Oxy::Architecture::X86_64 {
    class SimpleRegisterAllocator final : public RegisterAllocator {
    public:
        void allocate(MachineFunction* machine_func) override;

    private:
        struct LiveInterval {
            uint32_t vreg;
            uint32_t start;
            uint32_t end;
            bool hasRegister = false;
            X86_64::Register assignedRegister = (X86_64::Register)-1;
            bool spilled = false;
            int64_t spillOffset = 0;
        };

        std::unordered_map<uint32_t, LiveInterval> _liveIntervals;
        void calculateLiveIntervals(MachineFunction * machine_function);
        void substituteRegisters(MachineFunction * machine_function);
    };
}

#endif //OXYIR_SIMPLEREGISTERALLOCATOR_H
