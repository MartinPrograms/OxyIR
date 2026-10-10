#ifndef OXYIR_BASICBLOCK_H
#define OXYIR_BASICBLOCK_H

#include <vector>
#include <string>
#include <memory>

#include "Context.h"
#include "Value/Instruction.h"
#include "Value/Instructions/AllocaOperation.h"
#include "Value/Instructions/AsmOperation.h"
#include "Value/Instructions/BinaryOperation.h"
#include "Value/Instructions/BitCastOperation.h"
#include "Value/Instructions/CallOperation.h"
#include "Value/Instructions/CastOperation.h"
#include "Value/Instructions/CopyOperation.h"
#include "Value/Instructions/EqualityOperation.h"
#include "Value/Instructions/GetElementPointerOperation.h"
#include "Value/Instructions/JumpNotZeroOperation.h"
#include "Value/Instructions/JumpOperation.h"
#include "Value/Instructions/LoadOperation.h"
#include "Value/Instructions/PhiOperation.h"
#include "Value/Instructions/ReturnOperation.h"
#include "Value/Instructions/StoreOperation.h"
#include "Value/Instructions/UnaryOperation.h"
#include "Value/Instructions/VaArgOperation.h"
#include "Value/Instructions/VaCopy.h"
#include "Value/Instructions/VaStart.h"

namespace Oxy {
    class BasicBlock {
    public:
        BasicBlock(Context* context, std::string name);
        ~BasicBlock() = default;

        [[nodiscard]] const std::string& getName() const;
        [[nodiscard]] const std::vector<std::unique_ptr<Instruction>>& getInstructions() const;

        Instruction* addInstruction(std::unique_ptr<Instruction> instr) {
            return _instructions.emplace_back(std::move(instr)).get();
        }

        [[nodiscard]] bool isEmpty() const {
            return _instructions.empty();
        }

        [[nodiscard]] bool hasTerminator() const;

        Instructions::BinaryOperation* addBinary(Instructions::BinaryOperation::Opcode opcode, Instructions::BinaryOperation::Signedness signedness, Value* lhs, Value* rhs);
        Instructions::UnaryOperation* addUnary(Instructions::UnaryOperation::Opcode opcode, Value* operand);

        Instructions::EqualityOperation *addEquality(Instructions::EqualityOperation::Opcode opcode,
                                                     Instructions::EqualityOperation::Signedness signedness, Value *lhs,
                                                     Value *rhs);

        Instructions::CopyOperation* addCopy(Value* operand);
        Instructions::LoadOperation* addLoad(Type* type, Value* operand);
        Instructions::StoreOperation* addStore(Value* destination, Value* value);
        Instructions::AllocaOperation* addAllocate(Type* type, Value* count = nullptr, size_t alignment = 0);
        Instructions::GetElementPointerOperation* addGetElementPointer(Type* type, Value* operand, std::vector<Value*> indices);
        Instructions::CastOperation* addCast(Instructions::CastOperation::Opcode opcode, Instructions::CastOperation::Signedness signedness, Value* operand, Type* resultType);

        Instructions::CallOperation* addCall(Value* callee, std::vector<Value*> args);

        Instructions::JumpOperation* addJump(const BasicBlock* target);
        Instructions::JumpNotZeroOperation* addJumpNotZero(Value* condition, const BasicBlock* trueTarget, const BasicBlock* falseTarget);
        Instructions::ReturnOperation* addReturn(Value* returnValue = nullptr);

        Instructions::PhiOperation* addPhi(Type* type, std::vector<Instructions::PhiOperation::Incoming> incoming);
        Instructions::AsmOperation* addAsm(Type* outputType, std::string assemblyCode, std::vector<Instructions::AsmOperation::Operand> args);

        Instructions::VaArgOperation* addVaArg(Type* resultType, Value* vaListOperand);
        Instructions::VaStartOperation* addVaStart(Value* vaListOperand);
        Instructions::VaCopyOperation* addVaCopy(Value* dest, Value* src);

        Instructions::BitCastOperation* addBitCast(Value* operand, Type* resultType);

        Instructions::CustomInstruction* addCustom(std::unique_ptr<Instructions::CustomInstruction> instr);

    private:
        Context* _context;
        std::string _name;
        std::vector<std::unique_ptr<Instruction>> _instructions;
    };
} // Oxy

#endif //OXYIR_BASICBLOCK_H