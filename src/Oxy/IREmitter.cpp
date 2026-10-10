#include "IREmitter.h"

#include "Type/VariadicListType.h"
#include "Value/ArgumentValue.h"
#include "Value/Literal.h"
#include "Value/Instructions/BinaryOperation.h"
#include "Value/Instructions/ReturnOperation.h"

namespace Oxy {
    void IREmitter::emitModule(const Module &module) {
        _context = module.getContext();

        for (const auto& data : module.getGlobalVariables()) {
            emitGlobal(data.get());
        }

        for (const auto& func : module.getFunctions()) {
            emitFunction(func.get());
        }
    }

    void IREmitter::emitFunction(const Function *func) {
        // fn <type> <flags> $<name>(<type1> <arg1>, <type2> <arg2>, ...)
        // optional { <basic block 1> <basic block 2> ... } if the function is not external

        _output << "fn ";
        const auto& funcType = (FunctionType*) func->getType();
        emitType(funcType->getReturnType());
        uint32_t flags = func->getFlags();
        if (flags != 0) {
            if (flags & Function::Flags::Export) {
                _output << " export";
            }
            if (flags & Function::Flags::Inline) {
                _output << " inline";
            }
            if (flags & Function::Flags::Extern) {
                _output << " extern";
            }
        }

        _output << " $" << func->getName() << "(";
        for (size_t i = 0; i < funcType->getParamTypes().size(); i++) {
            if (i > 0) {
                _output << ", ";
            }
            emitType(funcType->getParamTypes()[i]);
            _output << " %arg" << i;
        }

        if (funcType->isVarArg()) {
            if (!funcType->getParamTypes().empty()) {
                _output << ", ";
            }
            _output << "...";
        }

        _output << ")";

        if (flags & Function::Flags::Extern) {
            _output << "\n";
            return;
        }

        _output << " {\n";

        for (const auto& block : func->getBasicBlocks()) {
            emitBasicBlock(block.get());
        }

        _output << "}\n";

        _localValueCounter = 0; // Reset local value counter for the next function
    }

    void IREmitter::emitBasicBlock(const BasicBlock *block) {
        // @<name>
        //   <instruction 1>
        //   <instruction 2>
        //   ...

        _output << "@" + block->getName() << "\n";
        for (const auto& instr : block->getInstructions()) {
            emitInstruction(instr.get());
        }
    }

    void IREmitter::emitInstruction(const Instruction *instr) {
        _output << "\t";
        if (auto ret = dynamic_cast<const Instructions::ReturnOperation*>(instr)) {
            // ret <type> [<value>] - if the return value is void, the type is omitted and no value is emitted
            _output << "ret";
            _output << " ";
            emitType(ret->getType());
            if (ret->getReturnValue()) {
                _output << " ";
                emitValue(ret->getReturnValue());
            }
            _output << "\n";
            return;
        }

        if (auto binary = dynamic_cast<const Instructions::BinaryOperation*>(instr)) {
            // %<result> = <opcode> <type> <value1>, <value2>
            std::string opcodeStr;
            switch (binary->getOpcode()) {
                case Instructions::BinaryOperation::Opcode::Add:
                    opcodeStr = "add";
                    break;
                case Instructions::BinaryOperation::Opcode::Sub:
                    opcodeStr = "sub";
                    break;
                case Instructions::BinaryOperation::Opcode::Mul:
                    opcodeStr = "mul";
                    break;
                case Instructions::BinaryOperation::Opcode::Div:
                    opcodeStr = "div";
                    break;
                case Instructions::BinaryOperation::Opcode::Rem:
                    opcodeStr = "rem";
                    break;
                case Instructions::BinaryOperation::Opcode::And:
                    opcodeStr = "and";
                    break;
                case Instructions::BinaryOperation::Opcode::Or:
                    opcodeStr = "or";
                    break;
                case Instructions::BinaryOperation::Opcode::Xor:
                    opcodeStr = "xor";
                    break;
                case Instructions::BinaryOperation::Opcode::Shl:
                    opcodeStr = "shl";
                    break;
                case Instructions::BinaryOperation::Opcode::Shr:
                    opcodeStr = "shr";
                    break;
            }

            // If the operation is signed, or unsigned and Add, Sub, Mul or Div add a sign suffix
            if (binary->getSignedness() != Instructions::BinaryOperation::Signedness::None) {
                if (binary->getOpcode() == Instructions::BinaryOperation::Opcode::Add ||
                    binary->getOpcode() == Instructions::BinaryOperation::Opcode::Sub ||
                    binary->getOpcode() == Instructions::BinaryOperation::Opcode::Mul ||
                    binary->getOpcode() == Instructions::BinaryOperation::Opcode::Rem ||
                    binary->getOpcode() == Instructions::BinaryOperation::Opcode::Div) {
                    opcodeStr += (binary->getSignedness() == Instructions::BinaryOperation::Signedness::Signed) ? "s" : "u";
                    }
            }

            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            _output << resultName << " = " << opcodeStr << " ";
            emitType(instr->getType());
            _output << " ";
            emitValue(binary->getLHS());
            _output << ", ";
            emitValue(binary->getRHS());
            _output << "\n";
            return;
        }

        if (auto unary = dynamic_cast<const Instructions::UnaryOperation*>(instr)) {
            // %<result> = <opcode> <type> <value>
            std::string opcodeStr;
            switch (unary->getOpcode()) {
                case Instructions::UnaryOperation::Opcode::Neg:
                    opcodeStr = "neg";
                    break;
                case Instructions::UnaryOperation::Opcode::Not:
                    opcodeStr = "not";
                    break;
            }

            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            _output << resultName << " = " << opcodeStr << " ";
            emitType(instr->getType());
            _output << " ";
            emitValue(unary->getOperand());
            _output << "\n";
            return;
        }

        if (auto equality = dynamic_cast<const Instructions::EqualityOperation*>(instr)) {
            // %<result> = <opcode> <type> <value1>, <value2>
            std::string opcodeStr;
            switch (equality->getOpcode()) {
                case Instructions::EqualityOperation::Opcode::Eq:
                    opcodeStr = "eq";
                    break;
                case Instructions::EqualityOperation::Opcode::Ne:
                    opcodeStr = "ne";
                    break;
                case Instructions::EqualityOperation::Opcode::Lt:
                    opcodeStr = "lt";
                    break;
                case Instructions::EqualityOperation::Opcode::Gt:
                    opcodeStr = "gt";
                    break;
                case Instructions::EqualityOperation::Opcode::Le:
                    opcodeStr = "le";
                    break;
                case Instructions::EqualityOperation::Opcode::Ge:
                    opcodeStr = "ge";
                    break;
            }

            // If the operation is signed, or unsigned and Lt, Gt, Le or Ge add a sign suffix
            if (equality->getSignedness() != Instructions::EqualityOperation::Signedness::None) {
                if (equality->getOpcode() == Instructions::EqualityOperation::Opcode::Lt ||
                    equality->getOpcode() == Instructions::EqualityOperation::Opcode::Gt ||
                    equality->getOpcode() == Instructions::EqualityOperation::Opcode::Le ||
                    equality->getOpcode() == Instructions::EqualityOperation::Opcode::Ge) {
                    opcodeStr += (equality->getSignedness() == Instructions::EqualityOperation::Signedness::Signed) ? "s" : "u";
                    }
            }

            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            _output << resultName << " = " << opcodeStr << " ";
            emitType(instr->getType());
            _output << " ";
            emitValue(equality->getLHS());
            _output << ", ";
            emitValue(equality->getRHS());
            _output << "\n";
            return;
        }

        if (auto load = dynamic_cast<const Instructions::LoadOperation*>(instr)) {
            // %<result> = load <type> <value>
            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            _output << resultName << " = load ";
            emitType(load->getType());
            _output << " ";
            emitValue(load->getOperand());
            _output << "\n";
            return;
        }

        if (auto store = dynamic_cast<const Instructions::StoreOperation*>(instr)) {
            // store <type> <destination>, <value>
            _output << "store ";
            emitType(store->getValue()->getType());
            _output << " ";
            emitValue(store->getDestination());
            _output << ", ";
            emitValue(store->getValue());
            _output << "\n";
            return;
        }

        if (auto alloca = dynamic_cast<const Instructions::AllocaOperation*>(instr)) {
            // %<result> = alloca <type>[, <count>]
            // if aligned its %<result> = alloca_align <type>, <alignment>[, <count>]
            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            if (alloca->getAlignment() > 0) {
                _output << resultName << " = alloca_align ";
                emitType(alloca->getType());
                _output << ", " << alloca->getAlignment();
            } else {
                _output << resultName << " = alloca ";
                emitType(alloca->getType());
            }

            if (alloca->getCount()) {
                _output << ", ";
                emitValue(alloca->getCount());
            }

            _output << "\n";
            return;
        }

        if (auto gep = dynamic_cast<const Instructions::GetElementPointerOperation*>(instr)) {
            // %<result> = gep <type> <value>, <index1>, <index2>, ...
            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            _output << resultName << " = gep ";
            emitType(gep->getType());
            _output << " ";
            emitValue(gep->getOperand());
            for (const auto& index : gep->getIndices()) {
                _output << ", ";
                emitValue(index);
            }
            _output << "\n";
            return;
        }

        if (auto cast = dynamic_cast<const Instructions::CastOperation*>(instr)) {
            // %<result> = <opcode> <result_type> <source_type> <value>

            std::string opcodeStr;
            switch (cast->getOpcode()) {
                case Instructions::CastOperation::Opcode::Trunc:
                    opcodeStr = "trunc";
                    break;
                case Instructions::CastOperation::Opcode::ZExt:
                    opcodeStr = "zext";
                    break;
                case Instructions::CastOperation::Opcode::SExt:
                    opcodeStr = "sext";
                    break;
                case Instructions::CastOperation::Opcode::FpTrunc:
                    opcodeStr = "fptrunc";
                    break;
                case Instructions::CastOperation::Opcode::FpExt:
                    opcodeStr = "fpext";
                    break;
                case Instructions::CastOperation::Opcode::PtrToInt:
                    opcodeStr = "ptrtoint";
                    break;
                case Instructions::CastOperation::Opcode::IntToPtr:
                    opcodeStr = "inttoptr";
                    break;
                case Instructions::CastOperation::Opcode::IntToFloat:
                    opcodeStr = "inttofloat";
                    break;
                case Instructions::CastOperation::Opcode::FloatToInt:
                    opcodeStr = "floattoint";
                    break;
            }

            if (cast->getSignedness() != Instructions::CastOperation::Signedness::None) {
                if (cast->getOpcode() == Instructions::CastOperation::Opcode::Trunc ||
                    cast->getOpcode() == Instructions::CastOperation::Opcode::ZExt ||
                    cast->getOpcode() == Instructions::CastOperation::Opcode::SExt ||
                    cast->getOpcode() == Instructions::CastOperation::Opcode::IntToFloat) {
                    opcodeStr += (cast->getSignedness() == Instructions::CastOperation::Signedness::Signed) ? "s" : "u";
                }
            }

            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            _output << resultName << " = " << opcodeStr << " ";
            emitType(cast->getType());
            _output << " ";
            emitType(cast->getOperand()->getType());
            _output << " ";
            emitValue(cast->getOperand());
            _output << "\n";
            return;
        }

        if (dynamic_cast<const Instructions::JumpOperation*>(instr)) {
            // jump <label>
            const auto* jump = dynamic_cast<const Instructions::JumpOperation*>(instr);
            _output << "jmp @" << jump->getTarget()->getName() << "\n";
            return;
        }

        if (dynamic_cast<const Instructions::JumpNotZeroOperation*>(instr)) {
            // jnz <value>, <true_label>, <false_label>
            const auto* jnz = dynamic_cast<const Instructions::JumpNotZeroOperation*>(instr);
            _output << "jnz ";
            emitValue(jnz->getCondition());
            _output << ", @" << jnz->getTrueTarget()->getName();
            _output << ", @" << jnz->getFalseTarget()->getName();
            _output << "\n";
            return;
        }

        if (auto call = dynamic_cast<const Instructions::CallOperation*>(instr)) {
            // %<result> = call <type> <callee>, <type1> <arg1>, <type2> <arg2>, ..., <typeN> <argN>
            if (((FunctionType*)call->getCallee()->getType())->getReturnType()->getKind() == Type::Kind::Void) {
                // If the return type is void, omit the result assignment
                _output << "call ";
            } else {
                std::string resultName = getNextLocalValueName();
                _valueNames[instr] = resultName;

                _output << resultName << " = call ";
            }

            emitType(((FunctionType*)call->getFunctionType())->getReturnType());
            _output << " ";
            emitValue(call->getCallee());

            bool hasVarargs = ((FunctionType*)call->getFunctionType())->isVarArg();
            bool emittedEllipsis = false;
            for (size_t i = 0; i < call->getArgs().size(); i++) {
                if (hasVarargs && i >= ((FunctionType*)call->getCallee()->getType())->getParamTypes().size() && !emittedEllipsis) {
                    _output << ", ...";
                    emittedEllipsis = true;
                }
                _output << ", ";
                emitType(call->getArgs()[i]->getType());
                _output << " ";
                emitValue(call->getArgs()[i]);
            }

            _output << "\n";
            return;
        }

        if (auto vaarg = dynamic_cast<const Instructions::VaArgOperation*>(instr)) {
            // %<result> = vaarg <type> <va_list>
            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            _output << resultName << " = vaarg ";
            emitType(vaarg->getType());
            _output << " ";
            emitValue(vaarg->getVaListOperand());
            _output << "\n";
            return;
        }

         if (auto vaStart = dynamic_cast<const Instructions::VaStartOperation*>(instr)) {
             // va_start <va_list>
             _output << "vastart" << " ";
             emitValue(vaStart->getVaListOperand());
             _output << "\n";
             return;
         }

        if (auto vaCopy = dynamic_cast<const Instructions::VaCopyOperation*>(instr)) {
            // va_copy <dest_va_list>, <src_va_list>
            _output << "vacopy ";
            emitValue(vaCopy->getDestVaListOperand());
            _output << ", ";
            emitValue(vaCopy->getSrcVaListOperand());
            _output << "\n";
            return;
        }

        if (auto bitcast = dynamic_cast<const Instructions::BitCastOperation*>(instr)) {
            // %<result> = bitcast <type> <value>
            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            _output << resultName << " = bitcast ";
            emitType(bitcast->getType());
            _output << " ";
            emitType(bitcast->getOperand()->getType());
            _output << " ";
            emitValue(bitcast->getOperand());
            _output << "\n";
            return;
        }

        if (auto copy = dynamic_cast<const Instructions::CopyOperation*>(instr)) {
            // %<result> = copy <type> <value>
            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            _output << resultName << " = copy ";
            emitType(copy->getType());
            _output << " ";
            emitValue(copy->getOperand());
            _output << "\n";
            return;
        }

        if (auto asmOp = dynamic_cast<const Instructions::AsmOperation*>(instr)) {
            // %<result> = asm <type> "assembly code", <type1> <arg1>, <type2> <arg2>, ...
            if (asmOp->getType()->getKind() != Type::Kind::Void) {
                std::string resultName = getNextLocalValueName();
                _valueNames[instr] = resultName;

                _output << resultName << " = ";
            }
            _output << "asm ";
            emitType(asmOp->getType());
            _output << " \"" << asmOp->getAssemblyCode() << "\"";
            for (const auto& arg : asmOp->getArgs()) {
                _output << ", ";
                emitType(arg.type);
                _output << " ";
                emitValue(arg.value);
            }
            _output << "\n";
            return;
        }

        if (auto phiOp = dynamic_cast<const Instructions::PhiOperation*>(instr)) {
            // %<result> = phi <type>, @<label1> <value1>, @label2 <value2>, ...
            std::string resultName = getNextLocalValueName();
            _valueNames[instr] = resultName;

            _output << resultName << " = phi ";
            emitType(phiOp->getType());

            for (const auto& incoming : phiOp->getIncoming()) {
                _output << ", @" << incoming.block->getName() << " ";
                emitValue(incoming.value);
            }

            _output << "\n";
            return;
        }

        if (auto customOp = dynamic_cast<const Instructions::CustomInstruction*>(instr)) {
            if (customOp->producesValue()) {
                std::string resultName = getNextLocalValueName();
                _valueNames[instr] = resultName;

                _output << resultName << " = ";
            }
            _output << customOp->getName() << " ";
            emitType(customOp->getType());
            for (const auto& operand : customOp->getOperands()) {
                _output << ", ";
                emitType(operand->getType());
                _output << " ";
                emitValue(operand);
            }
            _output << "\n";
            return;
        }

        auto name = typeid(*instr).name();
        _context->emitError("Unknown instruction type '" + std::string(name) + "' in IREmitter");
    }

    void IREmitter::emitType(const Type *type) {
        if (type == nullptr || dynamic_cast<const VoidType*>(type)) {
            _output << "void";
            return;
        }

        if (auto *primitive = dynamic_cast<const PrimitiveType*>(type)) {
            switch (primitive->getPrimitiveKind()) {
                case PrimitiveType::Kind::I1:
                    _output << "i1";
                    break;
                case PrimitiveType::Kind::I8:
                    _output << "i8";
                    break;
                case PrimitiveType::Kind::I16:
                    _output << "i16";
                    break;
                case PrimitiveType::Kind::I32:
                    _output << "i32";
                    break;
                case PrimitiveType::Kind::I64:
                    _output << "i64";
                    break;
                case PrimitiveType::Kind::F32:
                    _output << "f32";
                    break;
                case PrimitiveType::Kind::F64:
                    _output << "f64";
                    break;
            }
            return;
        }

        if (auto funcType = dynamic_cast<const FunctionType*>(type)) {
            // (paramType1, paramType2, ...) -> returnType
            _output << "(";
            for (size_t i = 0; i < funcType->getParamTypes().size(); i++) {
                if (i > 0) {
                    _output << ", ";
                }
                emitType(funcType->getParamTypes()[i]);
            }

            if (funcType->isVarArg()) {
                if (!funcType->getParamTypes().empty()) {
                    _output << ", ";
                }
                _output << "...";
            }

            _output << ") -> ";
            emitType(funcType->getReturnType());
            return;
        }

        if (auto structType = dynamic_cast<const StructType*>(type)) {
            auto flags = std::string();
            if (structType->getAlignment() != 0) {
                flags += " align " + std::to_string(structType->getAlignment());
            }

            _output << "struct" + flags + " { ";
            bool shouldEmitComma = false;
            for (const auto& fieldType : structType->getMemberTypes()) {
                if (shouldEmitComma) {
                    _output << ", ";
                }
                emitType(fieldType);
                _output << " ";
                shouldEmitComma = true;
            }
            _output << "}";
            return;
        }

        if (auto arrayType = dynamic_cast<const ArrayType*>(type)) {
            _output << "[";
            emitType(arrayType->getElementType());
            _output << ", " << arrayType->getSize() << "]";
            return;
        }

        if (dynamic_cast<const PointerType*>(type)) {
            _output << "ptr";
            return;
        }

        if (dynamic_cast<const VariadicListType*>(type)) {
            _output << "va_list";
            return;
        }

        auto name = typeid(*type).name();
        _context->emitError("Unknown type '" + std::string(name) + "' in IREmitter");
    }

    void IREmitter::emitValue(const Value *value) {
        if (auto literal = dynamic_cast<const Literal*>(value)) {
            if (std::holds_alternative<int64_t>(literal->getData())) {
                _output << std::get<int64_t>(literal->getData());
            } else if (std::holds_alternative<double>(literal->getData())) {
                _output << std::get<double>(literal->getData());
            }
            return;
        }

        if (auto argument = dynamic_cast<const ArgumentValue*>(value)) {
            _output << "%arg" << argument->getIndex();
            return;
        }

        // Check if the value exists in the value names map (for instructions)
        auto it = _valueNames.find(value);
        if (it != _valueNames.end()) {
            _output << it->second;
            return;
        }

        // Check if the value is a global
        if (auto data = dynamic_cast<const Data*>(value)) {
            _output << "$" << data->getName();
            return;
        }

        if (auto func = dynamic_cast<const Function*>(value)) {
            _output << "$" << func->getName();
            return;
        }

        auto name = typeid(*value).name();
        _context->emitError("Unknown value type '" + std::string(name) + "' in IREmitter");
    }

    void IREmitter::emitGlobal(const Data *data) {
        // data <type> <flags> $<name> = [0x00, 0x01, ...]
        // all data initializers are lowered to byte arrays!
        _output << "data ";
        emitType(data->getElementType());

        uint32_t flags = (uint32_t)data->getFlags();
        if (flags != (uint32_t)Data::Flags::None) {
            if ((flags & (uint32_t)Data::Flags::Export) != 0) {
                _output << " export";
            }
            if ((flags & (uint32_t)Data::Flags::ThreadLocal) != 0) {
                _output << " thread_local";
            }
            if ((flags & (uint32_t)Data::Flags::Mutable) != 0) {
                _output << " mutable";
            }
            if ((flags & (uint32_t)Data::Flags::Align) != 0) {
                _output << " align " << data->getAlignment();
            }
        }

        _output << " $" << data->getName() << " = [";
        const auto& init = data->getInitialValue();
        for (size_t i = 0; i < init.size(); i++) {
            if (i > 0) {
                _output << ", ";
            }
            _output << "0x" << std::hex << static_cast<int>(init[i]) << std::dec;
        }
        _output << "]\n";
    }
} // Oxy