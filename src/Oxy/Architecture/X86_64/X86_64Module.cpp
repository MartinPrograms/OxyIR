#include "X86_64Module.h"

#include "X86_64.h"
#include "Oxy/Value/ArgumentValue.h"
#include "../../Utilities.h"

void Oxy::Architecture::X86_64Module::lower() {
    const auto& functions = _module->getFunctions();

    for (const auto & function : functions) {
        _functions.emplace_back(std::make_unique<MachineFunction>(function->getName()));
    }

    for (size_t i = 0; i < functions.size(); i++) {
        lower_function(functions[i].get(), _functions[i].get());
    }

    auto debug_output = [&](std::vector<std::unique_ptr<MachineFunction>>& function_list) {
        for (const auto& func : function_list) {
            std::cout << "Function: " << func->name << "\n";
            // First emit the allocated stack slots for the function
            std::cout << "  Stack frame size: " << func->stackBase << " bytes\n";
            for (const auto& [alloca, slot] : _stackSlots) {
                std::cout << "\tvreg" << _vregTables[func->vregInfo][alloca] << " = [RBP" << (slot.offset < 0 ? " - " : " + ") << std::abs(slot.offset) << "]\n";
            }

            for (const auto& block : func->blocks) {
                std::cout << "  Block @" << block.first->getName() << ":\n";
                for (const auto& instr : block.second.instructions) {
                    std::cout << "    Opcode: " << (X86_64::Opcode)instr.opcode << ", Operands: ";
                    for (const auto& operand : instr.operands) {
                        switch (operand.kind) {
                            case MachineOperand::Kind::VirtualRegister:
                                std::cout << "vreg" << std::get<uint32_t>(operand.value) << " ";
                                break;
                            case MachineOperand::Kind::PhysicalRegister:
                                std::cout << "preg " << (X86_64::Register)std::get<uint32_t>(operand.value) << " ";
                                break;
                            case MachineOperand::Kind::Immediate:
                                std::cout << "imm " << std::get<int64_t>(operand.value) << " ";
                                break;
                            case MachineOperand::Kind::FloatValue:
                                std::cout << "float " << std::get<double>(operand.value) << " ";
                                break;
                            case MachineOperand::Kind::Block:
                                std::cout << "block @" << std::get<MachineBlock*>(operand.value)->id->getName() << " ";
                                break;
                            case MachineOperand::Kind::StackSlot: {
                                auto slot = std::get<StackSlot>(operand.value);
                                std::cout << "stack [preg " << (X86_64::Register)slot.base << (slot.offset < 0 ? " - " : " + ") << std::abs(slot.offset) << "] ";
                                break;
                            }
                            default:
                                std::cout << "unknown_operand ";
                        }
                    }
                    std::cout << "\n";
                }
            }
        }
    };

    debug_output(_functions);

    for (auto& func : _functions) {
        _register_allocator->allocate(func.get());
    }

    std::cout << "\n\nAfter register allocation:\n\n";

    debug_output(_functions);
}

size_t Oxy::Architecture::X86_64Module::getBlockIndex(Function *func, BasicBlock *get) {
    for (size_t i = 0; i < func->getBasicBlocks().size(); i++) {
        if (func->getBasicBlocks()[i].get() == get) {
            return i;
        }
    }

    _context->emitError("Block not found in function");
    return 0;
}

void Oxy::Architecture::X86_64Module::lower_function(Function *func, MachineFunction *machine_func) {
    _currentFunction = machine_func;
    _currentIRFunction = func;
    machine_func->name = func->getName();
    machine_func->vregInfo = (uint32_t)_vregTables.size();
    _vregTables.emplace_back();

    for (const auto& param : func->getArguments()) {
        _vregTables.back()[param.get()] = getVirtualRegister();
    }

    for (size_t i = 0; i < func->getBasicBlocks().size(); i++) {
        machine_func->blocks.emplace_back(func->getBasicBlocks()[i].get(), MachineBlock{func->getBasicBlocks()[i].get(), {}, {}, {}});
    }

    int64_t currentOffset = 0;
    for (const auto& block : func->getBasicBlocks()) {
        for (const auto& instr : block->getInstructions()) {
            if (auto* alloca = dynamic_cast<Instructions::AllocaOperation*>(instr.get())) {
                int64_t size = getTypeSize(alloca->getType());
                int64_t count = alloca->getCount()
                    ? std::get<int64_t>(dynamic_cast<Literal*>(alloca->getCount())->getData())
                    : 1;
                int64_t align = alloca->getAlignment() != 0 ? alloca->getAlignment() : size;
                currentOffset = (currentOffset + size * count + align - 1) & ~(align - 1);
                _allocaOffsets[alloca] = currentOffset; // RSP-relative, grows downward
            }
        }
    }

    int64_t frameSize = (currentOffset + 15) & ~15LL; // 16-byte align
    machine_func->stackBase = (uint32_t)frameSize;

    // Emit prologue into entry block
    auto* entryMachineBlock = &machine_func->blocks[0].second;
    if (frameSize > 0) {
        entryMachineBlock->instructions.insert(
            entryMachineBlock->instructions.begin(),
            MachineInstruction{
                (uint32_t)X86_64::Opcode::SUB, 0,
                {
                    MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RSP},
                    MachineOperand{MachineOperand::Kind::Immediate, 0, frameSize}
                }
            }
        );

        entryMachineBlock->instructions.insert(
            entryMachineBlock->instructions.begin(),
            MachineInstruction{
                (uint32_t)X86_64::Opcode::MOV, 0,
                {
                    MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RBP},
                    MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RSP}
                }
            }
        );

        entryMachineBlock->instructions.insert(
            entryMachineBlock->instructions.begin(),
            MachineInstruction{
                (uint32_t)X86_64::Opcode::PUSH, 0,
                {
                    MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RBP}
                }
            }
        );

        // Its in reverse order because we are inserting at the beginning of the block
    }

    for (const auto& block : func->getBasicBlocks()) {
        auto machine_block = &machine_func->blocks[getBlockIndex(func, block.get())].second;
        lower_block(block.get(), machine_block);
    }
}

void Oxy::Architecture::X86_64Module::lower_block(BasicBlock *block, MachineBlock *machine_block) {
    _currentBlock = machine_block;

    for (const auto& instr : block->getInstructions()) {
        lower_instruction(instr.get());
    }
}

void Oxy::Architecture::X86_64Module::lower_instruction(Instruction *instr) {
    instr->accept(this);
}

Oxy::Architecture::MachineOperand Oxy::Architecture::X86_64Module::resolveValue(Value *value) {
    if (auto *alloca = dynamic_cast<Instructions::AllocaOperation*>(value)) {
        return MachineOperand{MachineOperand::Kind::StackSlot, 0, _stackSlots[alloca]};
    }

    if (auto *instr = dynamic_cast<Instruction*>(value)) {
        auto [it, inserted] = getCurrentVregTable().try_emplace(instr, 0);
        if (inserted) it->second = getVirtualRegister();
        return MachineOperand{MachineOperand::Kind::VirtualRegister, 0, it->second};
    }

    if (auto *literal = dynamic_cast<Literal*>(value)) {
        if (((PrimitiveType*) literal->getType())->getPrimitiveKind() == PrimitiveType::Kind::F32 || ((PrimitiveType*) literal->getType())->getPrimitiveKind() == PrimitiveType::Kind::F64) {
            return MachineOperand{MachineOperand::Kind::FloatValue, 0, std::get<double>(literal->getData())};
        }

        return MachineOperand{MachineOperand::Kind::Immediate, 0, std::get<int64_t>(literal->getData())};
    }

    if (auto *arg = dynamic_cast<ArgumentValue*>(value)) {
        auto [it, inserted] = getCurrentVregTable().try_emplace(arg, 0);
        if (inserted) it->second = getVirtualRegister();
        return MachineOperand{MachineOperand::Kind::VirtualRegister, 0, it->second};
    }

    _context->emitError("Cannot resolve value to machine operand: " + std::string(typeid(*value).name()));
    return MachineOperand{MachineOperand::Kind::Immediate, 0, 0};
}

int64_t Oxy::Architecture::X86_64Module::getTypeSize(Type *get_type) {
    if (auto *primitiveType = dynamic_cast<PrimitiveType*>(get_type)) {
        switch (primitiveType->getPrimitiveKind()) {
            case PrimitiveType::Kind::I8:
                return 1;
            case PrimitiveType::Kind::I16:
                return 2;
            case PrimitiveType::Kind::I32:
                return 4;
            case PrimitiveType::Kind::I64:
                return 8;
            case PrimitiveType::Kind::F32:
                return 4;
            case PrimitiveType::Kind::F64:
                return 8;
            default:
                _context->emitError("Unknown primitive type");
                return 0;
        }
    }

    if (dynamic_cast<PointerType*>(get_type)) {
        return Oxy::Module::pointerSizes[_module->getArchitecture()];
    }

    _context->emitError("Cannot get size of non-primitive type");
    return 0;
}

void Oxy::Architecture::X86_64Module::visit(Instructions::AllocaOperation *instr) {
    getCurrentVregTable()[instr] = getVirtualRegister();
    int64_t offset = _allocaOffsets[instr];

    _stackSlots[instr] = StackSlot{(uint32_t)X86_64::Register::RBP, -offset};
}

void Oxy::Architecture::X86_64Module::visit(Instructions::AsmOperation *instr) {

}

void Oxy::Architecture::X86_64Module::visit(Instructions::BinaryOperation *instr) {
    getCurrentVregTable()[instr] = getVirtualRegister();

    auto src1 = resolveValue(instr->getLHS());
    auto src2 = resolveValue(instr->getRHS());
    auto dst = MachineOperand{MachineOperand::Kind::VirtualRegister, 0, getCurrentVregTable()[instr]};

    // Move src1 into dst.
    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, std::vector<MachineOperand>{src1, dst});

    uint32_t opcode = uint32_t(-1);
    switch (instr->getOpcode()) {
        case Instructions::BinaryOperation::Opcode::Add:
            opcode = (uint32_t)X86_64::Opcode::ADD;
            break;
        case Instructions::BinaryOperation::Opcode::Sub:
            opcode = (uint32_t)X86_64::Opcode::SUB;
            break;
        case Instructions::BinaryOperation::Opcode::Mul:
            opcode = (uint32_t)X86_64::Opcode::IMUL;
            break;
        case Instructions::BinaryOperation::Opcode::Div: {
            // Move to RAX, sign extend to RDX:RAX, then idiv, result is in RAX
            _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, std::vector<MachineOperand>{dst, MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RAX}});
            _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::CQO, 0, std::vector<MachineOperand>{});
            _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::IDIV, 0, std::vector<MachineOperand>{src2});

            // Move the result from RAX back to dst
            _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, std::vector<MachineOperand>{MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RAX}, dst});
            return;
        }
        case Instructions::BinaryOperation::Opcode::Rem: {
            // Move to RAX, sign extend to RDX:RAX, then idiv, result is in RDX
            _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, std::vector<MachineOperand>{dst, MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RAX}});
            _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::CQO, 0, std::vector<MachineOperand>{});
            _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::IDIV, 0, std::vector<MachineOperand>{src2});

            // Move the result from RDX back to dst
            _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, std::vector<MachineOperand>{MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RDX}, dst});
            return; // Skip the normal emit at the end since we already emitted the instructions for div and rem
        }
        case Instructions::BinaryOperation::Opcode::And:
            opcode = (uint32_t)X86_64::Opcode::AND;
            break;
        case Instructions::BinaryOperation::Opcode::Or:
            opcode = (uint32_t)X86_64::Opcode::OR;
            break;
        case Instructions::BinaryOperation::Opcode::Xor:
            opcode = (uint32_t)X86_64::Opcode::XOR;
            break;
        case Instructions::BinaryOperation::Opcode::Shl: {
            // Shift instructions in x86-64 require the shift amount to be in CL for variable shifts, so we need to move src2 into CL if it's not already there
            if (!(src2.kind == MachineOperand::Kind::PhysicalRegister && std::get<uint32_t>(src2.value) == (uint32_t)X86_64::Register::RCX)) {
                _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, std::vector<MachineOperand>{src2, MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RCX}});
            }
            opcode = (uint32_t)X86_64::Opcode::SHL;
            break;
        }
        case Instructions::BinaryOperation::Opcode::Shr: {
            // Shift instructions in x86-64 require the shift amount to be in CL for variable shifts, so we need to move src2 into CL if it's not already there
            if (!(src2.kind == MachineOperand::Kind::PhysicalRegister && std::get<uint32_t>(src2.value) == (uint32_t)X86_64::Register::RCX)) {
                _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, std::vector<MachineOperand>{src2, MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RCX}});
            }
            opcode = (uint32_t)X86_64::Opcode::SHR;
            break;
        }
        default:
            _context->emitError("Unknown binary operation opcode");
            return;
    }

    // Check if the operation is a floating point operation and adjust the opcode accordingly
    if ((dynamic_cast<PrimitiveType*>(instr->getType()))->getPrimitiveKind() == PrimitiveType::Kind::F32) {
        _context->emitError("Floating point operations are not yet implemented");
    } else if ((dynamic_cast<PrimitiveType*>(instr->getType()))->getPrimitiveKind() == PrimitiveType::Kind::F64) {
        _context->emitError("Floating point operations are not yet implemented");
    }

    if (opcode == uint32_t(-1)) {
        _context->emitError("Unsupported binary operation opcode");
        return;
    }

    std::vector<MachineOperand> operands = {
        dst, src2
    };

    _currentBlock->instructions.emplace_back(opcode, 0, operands);
}

void Oxy::Architecture::X86_64Module::visit(Instructions::BitCastOperation *instr) {
}

void Oxy::Architecture::X86_64Module::visit(Instructions::CallOperation *instr) {
}

void Oxy::Architecture::X86_64Module::visit(Instructions::CastOperation *instr) {
}

void Oxy::Architecture::X86_64Module::visit(Instructions::CopyOperation *instr) {
    getCurrentVregTable()[instr] = getVirtualRegister();

    std::vector<MachineOperand> operands = {
        resolveValue(instr->getOperand()),
        MachineOperand{MachineOperand::Kind::VirtualRegister, 0, getCurrentVregTable()[instr]}
    };

    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, operands);
}

void Oxy::Architecture::X86_64Module::visit(Instructions::CustomInstruction *instr) {
}

void Oxy::Architecture::X86_64Module::visit(Instructions::EqualityOperation *instr) {
    auto opcode = instr->getOpcode();
    auto signedness = instr->getSignedness();
    auto lhs = instr->getLHS();
    auto rhs = instr->getRHS();

    getCurrentVregTable()[instr] = getVirtualRegister();

    uint32_t x86Opcode = uint32_t(-1);
    switch (opcode) {
        case Instructions::EqualityOperation::Opcode::Eq:
            x86Opcode = (uint32_t)X86_64::Opcode::SETE;
            break;
        case Instructions::EqualityOperation::Opcode::Ne:
            x86Opcode = (uint32_t)X86_64::Opcode::SETNE;
            break;
        case Instructions::EqualityOperation::Opcode::Lt:
            x86Opcode = (signedness == Instructions::EqualityOperation::Signedness::Signed) ? (uint32_t)X86_64::Opcode::SETL
            : (uint32_t)X86_64::Opcode::SETB;
            break;
        case Instructions::EqualityOperation::Opcode::Gt:
            x86Opcode = (signedness == Instructions::EqualityOperation::Signedness::Signed) ? (uint32_t)X86_64::Opcode::SETG : (uint32_t)X86_64::Opcode::SETA;
            break;
        case Instructions::EqualityOperation::Opcode::Le:
            x86Opcode = (signedness == Instructions::EqualityOperation::Signedness::Signed) ? (uint32_t)X86_64::Opcode::SETLE : (uint32_t)X86_64::Opcode::SETBE;
            break;
        case Instructions::EqualityOperation::Opcode::Ge:
            x86Opcode = (signedness == Instructions::EqualityOperation::Signedness::Signed) ? (uint32_t)X86_64::Opcode::SETGE : (uint32_t)X86_64::Opcode::SETAE;
            break;
        default:
            _context->emitError("Unknown equality operation opcode");
            return;
    }

    // First we do a comp and set the flags, then we do a setcc instruction to set the result byte based on the flags
    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::CMP, 0, std::vector<MachineOperand>{resolveValue(lhs), resolveValue(rhs)});
    _currentBlock->instructions.emplace_back(x86Opcode, 0, std::vector<MachineOperand>{MachineOperand{MachineOperand::Kind::VirtualRegister, 0, getCurrentVregTable()[instr]}});
}

void Oxy::Architecture::X86_64Module::visit(Instructions::GetElementPointerOperation *instr) {
}

void Oxy::Architecture::X86_64Module::visit(Instructions::JumpNotZeroOperation *instr) {
    auto trueTarget = instr->getTrueTarget();
    auto falseTarget = instr->getFalseTarget();
    auto from = _currentBlock;
    auto trueTo = &_currentFunction->blocks[getBlockIndex(_currentIRFunction, (BasicBlock*)trueTarget)].second;
    auto falseTo = &_currentFunction->blocks[getBlockIndex(_currentIRFunction, (BasicBlock*)falseTarget)].second;
    from->successors.push_back((BasicBlock*)trueTarget);
    from->successors.push_back((BasicBlock*)falseTarget);
    trueTo->predecessors.push_back(from->id);
    falseTo->predecessors.push_back(from->id);

    // First we do a comp against zero and set the flags, then we do a conditional jump based on the flags
    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::CMP, 0, std::vector<MachineOperand>{resolveValue(instr->getCondition()), MachineOperand{MachineOperand::Kind::Immediate, 0, 0}});
    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::JNE, 0, std::vector<MachineOperand>{MachineOperand{MachineOperand::Kind::Block, 0, trueTo}});
    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::JMP, 0, std::vector<MachineOperand>{MachineOperand{MachineOperand::Kind::Block, 0, falseTo}});
}

void Oxy::Architecture::X86_64Module::visit(Instructions::JumpOperation *instr) {
    auto target = instr->getTarget();
    auto from = _currentBlock;
    auto to = &_currentFunction->blocks[getBlockIndex(_currentIRFunction, (BasicBlock*)target)].second;
    from->successors.push_back((BasicBlock*)target);
    to->predecessors.push_back(from->id);

    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::JMP, 0, std::vector<MachineOperand>{MachineOperand{MachineOperand::Kind::Block, 0, to}});
}

void Oxy::Architecture::X86_64Module::visit(Instructions::LoadOperation *instr) {
    getCurrentVregTable()[instr] = getVirtualRegister();

    std::vector<MachineOperand> operands = {
        resolveValue(instr->getOperand()),
        MachineOperand{MachineOperand::Kind::VirtualRegister, 0, getCurrentVregTable()[instr]}
    };

    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, operands);
}

void Oxy::Architecture::X86_64Module::visit(Instructions::PhiOperation *instr) {
    // A phi must be the first instruction in a block, which is already guaranteed no need to check here, however we must check if this block has predecessors, if not it is not valid.
    if (_currentBlock->predecessors.empty()) {
        _context->emitError("Phi operation cannot be in a block with no predecessors");
        return;
    }

    auto incoming = instr->getIncoming();
    for (const auto& inc : incoming) {
        // For each incoming value, we need to insert a move instruction at the end of the corresponding predecessor block to move the value into the virtual register assigned to the phi node.
        auto predBlock = &_currentFunction->blocks[getBlockIndex(_currentIRFunction, inc.block)].second;
        auto& instrs = predBlock->instructions;
        instrs.insert(std::prev(instrs.end()),MachineInstruction((uint32_t)X86_64::Opcode::MOV, 0, std::vector<MachineOperand>{
            resolveValue(inc.value),
            MachineOperand{MachineOperand::Kind::VirtualRegister, 0, getCurrentVregTable()[instr]}
        }));
    }

    // thats it, the phi node itself does not need to emit any instructions, it is just a marker for the codegen phase to know that it needs to move values into the correct virtual register based on the control flow path taken to reach the current block
}

void Oxy::Architecture::X86_64Module::visit(Instructions::ReturnOperation *instr) {
    std::vector<MachineOperand> operands;
    if (instr->getReturnValue() != nullptr) {
        operands.push_back(resolveValue(instr->getReturnValue()));
    }

    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, std::vector {
        MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RSP},
        MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RBP}
    });

    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::POP, 0, std::vector {
        MachineOperand{MachineOperand::Kind::PhysicalRegister, 0, (uint32_t)X86_64::Register::RBP}
    });

    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::RET, 0, operands);
}

void Oxy::Architecture::X86_64Module::visit(Instructions::StoreOperation *instr) {
    std::vector<MachineOperand> operands = {
        resolveValue(instr->getValue()),
        resolveValue(instr->getDestination())
    };

    _currentBlock->instructions.emplace_back((uint32_t)X86_64::Opcode::MOV, 0, operands);
}

void Oxy::Architecture::X86_64Module::visit(Instructions::UnaryOperation *instr) {
    auto opcode = instr->getOpcode();
    auto operand = instr->getOperand();

    getCurrentVregTable()[instr] = getVirtualRegister();

    uint32_t x86Opcode = uint32_t(-1);
    switch (opcode) {
        case Instructions::UnaryOperation::Opcode::Neg:
            x86Opcode = (uint32_t)X86_64::Opcode::NEG;
            break;
        case Instructions::UnaryOperation::Opcode::Not:
            x86Opcode = (uint32_t)X86_64::Opcode::NOT;
            break;
        default:
            _context->emitError("Unknown unary operation opcode");
            return;
    }

    std::vector<MachineOperand> operands = {
        resolveValue(operand),
        MachineOperand{MachineOperand::Kind::VirtualRegister, 0, getCurrentVregTable()[instr]}
    };

    _currentBlock->instructions.emplace_back(x86Opcode, 0, operands);
}

void Oxy::Architecture::X86_64Module::visit(Instructions::VaArgOperation *instr) {
}

void Oxy::Architecture::X86_64Module::visit(Instructions::VaCopyOperation *instr) {
}

void Oxy::Architecture::X86_64Module::visit(Instructions::VaStartOperation *instr) {
}
