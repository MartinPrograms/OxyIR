#include "SimpleRegisterAllocator.h"
#include "../../Utilities.h"
namespace Oxy::Architecture::X86_64 {
    // Simple linear scan register allocator
    void SimpleRegisterAllocator::allocate(MachineFunction *machine_func) {
        _liveIntervals.clear();

        calculateLiveIntervals(machine_func);
        substituteRegisters(machine_func);
    }

    void SimpleRegisterAllocator::calculateLiveIntervals(MachineFunction *machine_function) {
        uint32_t index = 0;

        for (auto& [bb, block] : machine_function->blocks) {
            for (auto& instr : block.instructions) {
                for (auto& operand : instr.operands) {
                    if (operand.kind != MachineOperand::Kind::VirtualRegister) continue;
                    uint32_t vreg = std::get<uint32_t>(operand.value);
                    auto [it, inserted] = _liveIntervals.try_emplace(vreg, LiveInterval{vreg, index, index});
                    if (!inserted)
                        it->second.end = index; // extend to latest use
                }
                index++;
            }
        }
    }

    void SimpleRegisterAllocator::substituteRegisters(MachineFunction *machine_function) {
        // Go through each live interval and assign a physical register if possible, otherwise spill a register to the stack
        std::vector<X86_64::Register> availableRegisters = {
            X86_64::Register::R8, X86_64::Register::R9, X86_64::Register::R10, X86_64::Register::R11
        };

        std::vector<LiveInterval*> sorted;
        for (auto& [vreg, interval] : _liveIntervals)
            sorted.push_back(&interval);
        std::sort(sorted.begin(), sorted.end(),
            [](const LiveInterval* a, const LiveInterval* b) { return a->start < b->start; });

        std::vector<LiveInterval*> active; // currently assigned intervals

        for (LiveInterval* interval : sorted) {
            // Expire old intervals
            active.erase(std::remove_if(active.begin(), active.end(),
                [interval](LiveInterval* activeInterval) { return activeInterval->end < interval->start; }),
                active.end());

            if (active.size() == availableRegisters.size()) {
                // No registers available, spill the interval with the furthest end point
                auto spill = std::max_element(active.begin(), active.end(),
                    [](LiveInterval* a, LiveInterval* b) { return a->end < b->end; });
                if ((*spill)->end > interval->end) {
                    // Spill the active interval
                    (*spill)->spilled = true;
                    (*spill)->assignedRegister = (X86_64::Register)-1;
                    interval->assignedRegister = availableRegisters[active.size() - 1];
                    active.push_back(interval);
                    std::sort(active.begin(), active.end(),
                        [](const LiveInterval* a, const LiveInterval* b) { return a->end < b->end; });
                } else {
                    // Spill the current interval
                    interval->spilled = true;
                }
            } else {
                // Assign a register to the current interval
                interval->assignedRegister = availableRegisters[active.size()];
                active.push_back(interval);
                std::sort(active.begin(), active.end(),
                    [](const LiveInterval* a, const LiveInterval* b) { return a->end < b->end; });
            }
        }

        // Now we have assigned registers or spilled all virtual registers, we need to go through the machine function and substitute virtual registers with physical registers or stack slots as needed
        for (auto& [bb, block] : machine_function->blocks) {
            for (auto& instr : block.instructions) {
                for (auto& operand : instr.operands) {
                    if (operand.kind != MachineOperand::Kind::VirtualRegister) continue;
                    uint32_t vreg = std::get<uint32_t>(operand.value);
                    auto& interval = _liveIntervals[vreg];
                    if (interval.spilled) {
                        // Replace with stack slot operand
                        operand.kind = MachineOperand::Kind::StackSlot;
                        operand.value = StackSlot{(uint32_t)X86_64::Register::RBP, -interval.spillOffset};
                        interval.spillOffset += 8; // Assuming 8 byte stack slots, this should be aligned to the largest type we need to spill
                    } else {
                        // Replace with physical register operand
                        operand.kind = MachineOperand::Kind::PhysicalRegister;
                        operand.value = (uint32_t)interval.assignedRegister;
                    }
                }
            }
        }

        // Debug print
        for (const auto& [vreg, interval] : _liveIntervals) {
            std::cout << "vreg" << vreg << ": [" << interval.start << ", " << interval.end << "]";
            if (interval.spilled) {
                std::cout << " spilled";
            } else {
                std::cout << " assigned to " << (X86_64::Register)interval.assignedRegister;
            }
            std::cout << "\n";
        }
    }
}
