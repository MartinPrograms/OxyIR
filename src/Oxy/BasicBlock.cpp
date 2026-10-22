#include "BasicBlock.h"

#include "Value/Instructions/ReturnOperation.h"

namespace Oxy {
    BasicBlock::BasicBlock(Context* context, std::string name) {
        _name = std::move(name);
        _context = context;
    }

    const std::string &BasicBlock::getName() const {
        return _name;
    }

    const std::vector<std::unique_ptr<Instruction>> & BasicBlock::getInstructions() const {
        return _instructions;
    }

    bool BasicBlock::hasTerminator() const {
        if (_instructions.empty()) return false;

        for (const auto& instr : _instructions) {
            if (dynamic_cast<const Instructions::ReturnOperation*>(instr.get()) != nullptr) {
                return true;
            }
            if (dynamic_cast<const Instructions::JumpOperation*>(instr.get()) != nullptr) {
                return true;
            }
            if (dynamic_cast<const Instructions::JumpNotZeroOperation*>(instr.get()) != nullptr) {
                return true;
            }
        }

        return false;
    }

    Instructions::BinaryOperation * BasicBlock::addBinary(Instructions::BinaryOperation::Opcode opcode, Instructions::BinaryOperation::Signedness signedness, Value *lhs,
        Value *rhs) {
        if (hasTerminator())
            _context->emitError("Cannot add binary operation to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::BinaryOperation>(opcode, signedness, lhs, rhs);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::UnaryOperation * BasicBlock::addUnary(Instructions::UnaryOperation::Opcode opcode, Value *operand) {
        if (hasTerminator())
            _context->emitError("Cannot add unary operation to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::UnaryOperation>(opcode, operand);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::ReturnOperation * BasicBlock::addReturn(Value *returnValue) {
        if (hasTerminator())
            _context->emitError("Cannot add return instruction to block '" + getName() + "' because it already has a terminator instruction");

        auto instr = std::make_unique<Instructions::ReturnOperation>(returnValue);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::PhiOperation * BasicBlock::addPhi(Type *type,
        std::vector<Instructions::PhiOperation::Incoming> incoming) {
        if (hasTerminator())
            _context->emitError("Cannot add phi instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::PhiOperation>(type, std::move(incoming));
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::AsmOperation * BasicBlock::addAsm(Type *outputType, std::string assemblyCode,
        std::vector<Instructions::AsmOperation::Operand> args) {
        if (hasTerminator())
            _context->emitError("Cannot add asm instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::AsmOperation>(outputType, std::move(assemblyCode), std::move(args));
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::VaArgOperation * BasicBlock::addVaArg(Type *resultType, Value *vaListOperand) {
        if (hasTerminator())
            _context->emitError("Cannot add vaarg instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::VaArgOperation>(resultType, vaListOperand);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::VaStartOperation * BasicBlock::addVaStart(Value *vaListOperand) {
        if (hasTerminator())
            _context->emitError("Cannot add vaarg instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::VaStartOperation>(vaListOperand);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::VaCopyOperation * BasicBlock::addVaCopy(Value *dest, Value *src) {
        if (hasTerminator())
            _context->emitError("Cannot add vacopy instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::VaCopyOperation>(dest, src);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::BitCastOperation * BasicBlock::addBitCast(Value *operand, Type *resultType) {
        if (hasTerminator())
            _context->emitError("Cannot add bitcast instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::BitCastOperation>(operand, resultType);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::CustomInstruction * BasicBlock::addCustom(std::unique_ptr<Instructions::CustomInstruction> instr) {
        if (hasTerminator())
            _context->emitError("Cannot add custom instruction to block '" + getName() + "' because it has been terminated");

        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::EqualityOperation *BasicBlock::addEquality(Instructions::EqualityOperation::Opcode opcode,
                                                             Instructions::EqualityOperation::Signedness signedness,
                                                             Value *lhs, Value *rhs) {
        if (hasTerminator())
            _context->emitError("Cannot add equality operation to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::EqualityOperation>(opcode, signedness, lhs, rhs);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::CopyOperation * BasicBlock::addCopy(Value *operand) {
        if (hasTerminator())
            _context->emitError("Cannot add copy instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::CopyOperation>(operand);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::LoadOperation * BasicBlock::addLoad(Type *type, Value *operand) {
        if (hasTerminator())
            _context->emitError("Cannot add load instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::LoadOperation>(type, operand);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::StoreOperation * BasicBlock::addStore(Value *to, Value *from) {
        if (hasTerminator())
            _context->emitError("Cannot add store instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::StoreOperation>(to, from);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::AllocaOperation * BasicBlock::addAllocate(Type *type, Value *count, size_t alignment) {
        if (hasTerminator())
            _context->emitError("Cannot add alloca instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::AllocaOperation>(type, count, alignment);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::GetElementPointerOperation * BasicBlock::addGetElementPointer(Type *type, Value *operand,
        std::vector<Value *> indices) {
        if (hasTerminator())
            _context->emitError("Cannot add getelementpointer instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::GetElementPointerOperation>(type, operand, std::move(indices));
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::CastOperation * BasicBlock::addCast(Instructions::CastOperation::Opcode opcode,
        Instructions::CastOperation::Signedness signedness, Value *operand, Type *resultType) {
        if (hasTerminator())
            _context->emitError("Cannot add cast instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::CastOperation>(opcode, signedness, operand, resultType);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::CallOperation * BasicBlock::addCall(Value *callee, FunctionType* functionType, std::vector<Value *> args) {
        if (hasTerminator())
            _context->emitError("Cannot add call instruction to block '" + getName() + "' because it has been terminated");

        auto instr = std::make_unique<Instructions::CallOperation>(callee, functionType, std::move(args));
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::JumpOperation * BasicBlock::addJump(const BasicBlock *target) {
        if (hasTerminator())
            _context->emitError("Cannot add jump instruction to block '" + getName() + "' because it already has a terminator instruction");

        auto instr = std::make_unique<Instructions::JumpOperation>(target);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }

    Instructions::JumpNotZeroOperation * BasicBlock::addJumpNotZero(Value *condition, const BasicBlock *trueTarget,
        const BasicBlock *falseTarget) {
        if (hasTerminator())
            _context->emitError("Cannot add jump not zero instruction to block '" + getName() + "' because it already has a terminator instruction");

        auto instr = std::make_unique<Instructions::JumpNotZeroOperation>(condition, trueTarget, falseTarget);
        auto ptr = instr.get();
        _instructions.push_back(std::move(instr));
        return ptr;
    }
} // Oxy