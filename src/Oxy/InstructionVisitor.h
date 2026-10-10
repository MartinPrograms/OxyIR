#ifndef OXYIR_INSTRUCTIONVISITOR_H
#define OXYIR_INSTRUCTIONVISITOR_H

namespace Oxy {
    namespace Instructions {
        class AllocaOperation;
        class AsmOperation;
        class BinaryOperation;
        class BitCastOperation;
        class CallOperation;
        class CastOperation;
        class CopyOperation;
        class CustomInstruction;
        class EqualityOperation;
        class GetElementPointerOperation;
        class JumpNotZeroOperation;
        class JumpOperation;
        class LoadOperation;
        class PhiOperation;
        class ReturnOperation;
        class StoreOperation;
        class UnaryOperation;
        class VaArgOperation;
        class VaCopyOperation;
        class VaStartOperation;
    }
}

namespace Oxy {
    // Visitor pattern for instructions, allows to visit different types of instructions without knowing their concrete types.
    class InstructionVisitor {
    public:
        virtual ~InstructionVisitor() = default;
        virtual void visit(Instructions::AllocaOperation* instr) = 0;
        virtual void visit(Instructions::AsmOperation* instr) = 0;
        virtual void visit(Instructions::BinaryOperation* instr) = 0;
        virtual void visit(Instructions::BitCastOperation* instr) = 0;
        virtual void visit(Instructions::CallOperation* instr) = 0;
        virtual void visit(Instructions::CastOperation* instr) = 0;
        virtual void visit(Instructions::CopyOperation* instr) = 0;
        virtual void visit(Instructions::CustomInstruction* instr) = 0;
        virtual void visit(Instructions::EqualityOperation* instr) = 0;
        virtual void visit(Instructions::GetElementPointerOperation* instr) = 0;
        virtual void visit(Instructions::JumpNotZeroOperation* instr) = 0;
        virtual void visit(Instructions::JumpOperation* instr) = 0;
        virtual void visit(Instructions::LoadOperation* instr) = 0;
        virtual void visit(Instructions::PhiOperation* instr) = 0;
        virtual void visit(Instructions::ReturnOperation* instr) = 0;
        virtual void visit(Instructions::StoreOperation* instr) = 0;
        virtual void visit(Instructions::UnaryOperation* instr) = 0;
        virtual void visit(Instructions::VaArgOperation* instr) = 0;
        virtual void visit(Instructions::VaCopyOperation* instr) = 0;
        virtual void visit(Instructions::VaStartOperation* instr) = 0;
    };
}

#endif //OXYIR_INSTRUCTIONVISITOR_H
