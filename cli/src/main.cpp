#include <iostream>
#include <ostream>
#include <Oxy/Module.h>

#include "Oxy/IREmitter.h"
#include "Oxy/Value/Literal.h"
#include "Oxy/Value/Instructions/BinaryOperation.h"
#include "Oxy/Value/Instructions/EqualityOperation.h"

using namespace Oxy;
using namespace Instructions;

void basic(Context& context, Module& module) {
    int32_t globalVarValue = 12345;
    auto global = module.addGlobalVariable(context.i32(), "globalVar", Data::Flags::Export, 4, context.toBytes(&globalVarValue, sizeof(globalVarValue)));

    const char* formatStr = "Result: %d\n";
    auto formatStrData = context.toBytes((void*)formatStr, strlen(formatStr) + 1); // +1 for null terminator
    auto formatStrGlobal = module.addGlobalVariable(context.arrayType(context.i8(), strlen(formatStr) + 1), "formatStr", Data::Flags::None, 1, formatStrData);

    auto printfFuncType = context.functionType(context.i32(), {context.ptr()}, FunctionType::CallingConvention::C, true);
    auto printfFunc = module.addFunction(printfFuncType, "printf", Function::Flags::Extern);

    auto funcType = context.functionType(context.i32(), {context.i32(), context.ptr()}, FunctionType::CallingConvention::C, false);
    auto func = module.addFunction(funcType, "main");

    auto block = func->addEntryBlock();
    auto fourtyTwo = context.literal(context.i32(), 42);
    auto fiftyfive = context.literal(context.i32(), 55);
    auto onehundred = context.literal(context.i32(), 100);

    auto addInstr = block->addBinary(BinaryOperation::Opcode::Add, BinaryOperation::Signedness::Signed, fourtyTwo, fiftyfive);
    auto subInstr = block->addBinary(BinaryOperation::Opcode::Sub, BinaryOperation::Signedness::Signed, addInstr, onehundred);
    auto negated = block->addUnary(UnaryOperation::Opcode::Neg, subInstr);
    auto notInstr = block->addUnary(UnaryOperation::Opcode::Not, negated);
    auto loadedGlobal = block->addLoad(context.i32(), global);
    auto equality = block->addEquality(EqualityOperation::Opcode::Gt, EqualityOperation::Signedness::Signed, notInstr, loadedGlobal);
    block->addStore(global, fourtyTwo);

    auto allocated = block->addAllocate(context.i32(), fourtyTwo, 16); // allocate an i32 on the stack with count = 42 and alignment = 16
    block->addStore(allocated, fiftyfive);

    auto fifthIndex = context.literal(context.i32(), 5);
    auto gep = block->addGetElementPointer(context.ptr(), allocated, {fifthIndex}); // get a pointer to the 5th element of the allocated array
    block->addStore(gep, onehundred);

    auto truncated = block->addCast(CastOperation::Opcode::Trunc, CastOperation::Signedness::Signed, onehundred, context.i16()); // truncate the i32 literal 100 to an i16
    auto extended = block->addCast(CastOperation::Opcode::ZExt, CastOperation::Signedness::Unsigned, truncated, context.i64()); // zero-extend the truncated i16 back to an i64
    auto floatingPointNumber = block->addCast(CastOperation::Opcode::IntToFloat, CastOperation::Signedness::Unsigned, extended, context.f64()); // extend the i64 to an f64
    auto float32 = block->addCast(CastOperation::Opcode::FpTrunc, CastOperation::Signedness::None, floatingPointNumber, context.f32()); // truncate the f64 to an f32
    auto intFromFloat = block->addCast(CastOperation::Opcode::FloatToInt, CastOperation::Signedness::Signed, float32, context.i32()); // convert the f32 back to an i32
    auto pointerFromInt = block->addCast(CastOperation::Opcode::IntToPtr, CastOperation::Signedness::None, intFromFloat, context.ptr()); // convert the i32 to a pointer
    auto intFromPointer = block->addCast(CastOperation::Opcode::PtrToInt, CastOperation::Signedness::None, pointerFromInt, context.i64()); // convert the pointer back to an integer (resulting in an i64)

    auto upcastGlobal = block->addCast(CastOperation::Opcode::ZExt, CastOperation::Signedness::Unsigned, loadedGlobal, context.i64()); // zero-extend the loaded global variable to an i64 to compare it with the integer obtained from the pointer cast
    auto intFromPointerGreaterThanLoadedGlobal = block->addEquality(EqualityOperation::Opcode::Gt, EqualityOperation::Signedness::Signed, intFromPointer, upcastGlobal); // compare the integer obtained from the pointer cast to the loaded global variable

    auto true_block = func->addBlock("true_block");
    auto false_block = func->addBlock("false_block");

    block->addJumpNotZero(intFromPointerGreaterThanLoadedGlobal, true_block, false_block);

    true_block->addCall(printfFunc, printfFuncType, {formatStrGlobal, intFromPointerGreaterThanLoadedGlobal});

    true_block->addReturn(fourtyTwo);
    false_block->addReturn(equality);
}

void vaarg(Context& context, Module& module) {
    auto funcType = context.functionType(context.i32(), {}, FunctionType::CallingConvention::C, true);
    auto func = module.addFunction(funcType, "varargFunc");

    auto block = func->addEntryBlock();
    auto va_list = block->addAllocate(context.vaListType());
    auto va_list2 = block->addAllocate(context.vaListType());
    block->addVaStart(va_list);
    block->addVaCopy(va_list2, va_list);

    auto arg0 = block->addVaArg(context.i32(), va_list);
    auto arg1 = block->addVaArg(context.f64(), va_list);
    auto arg2 = block->addVaArg(context.ptr(), va_list);
    auto arg4 = block->addVaArg(context.i32(), va_list);

    auto bitcasted = block->addBitCast(arg1, context.i64()); // bitcast the f64 argument to an i64 to demonstrate that we can operate on the va_arg results
    auto truncated = block->addCast(CastOperation::Opcode::Trunc, CastOperation::Signedness::Signed, bitcasted, context.i32()); // truncate the bitcasted i64 to an i32
    auto copied = block->addCopy(truncated);

    block->addAsm(context.voidType(), "nop", {});

    block->addVaCopy(va_list, va_list2); // copy back the va_list to the original one

    auto true_block =func->addBlock("true_block");
    auto false_block =func->addBlock("false_block");

    // Check if copied is above 100, if so jump to true_block, otherwise jump to false_block
    auto condition = block->addEquality(EqualityOperation::Opcode::Gt, EqualityOperation::Signedness::Signed, copied, context.literal(context.i32(), 100));
    block->addJumpNotZero(condition, true_block, false_block);

    auto true_allocated = true_block->addAllocate(context.i32(), context.literal(context.i32(), 10)); // allocate an array of 10 i32s on the stack in the true block
    auto false_allocated = false_block->addAllocate(context.i32(), context.literal(context.i32(), 20)); // allocate an array of 20 i32s on the stack in the false block

    auto after_block = func->addBlock("after_block");
    true_block->addJump(after_block);
    false_block->addJump(after_block);

    auto result = after_block->addPhi(context.i32(), {{true_allocated, true_block}, {false_allocated, false_block}}); // use a phi node to select the correct allocated array based on which block we came from
    after_block->addReturn(result);
}

class VolatileLoad : public CustomInstruction {
public:
    // XXX.load <type> <pointer>
    // Where XXX is the instruction set
    explicit VolatileLoad(std::string identifier, Type* type, Value* pointer) : CustomInstruction(std::move(identifier), {pointer}, type, true), _pointer(pointer) {}

    [[nodiscard]] Value* getPointer() const {
        return _pointer;
    }

    static constexpr const char* Name = "load";

private:
    Value* _pointer;
};

class VolatileStore : public CustomInstruction {
public:
    // XXX.store <type> <pointer> <value>
    // Where XXX is the instruction set
    // Unfortunately custom instructions do not have , or other syntax support, as they emit as a single token followed by the operands.
    explicit VolatileStore(std::string identifier, Type* type, Value* pointer,
                            Value* value) : CustomInstruction(std::move(identifier), {pointer, value}, type, false), _pointer(pointer), _value(value) {}

    [[nodiscard]] Value* getPointer() const {
        return _pointer;
    }

    [[nodiscard]] Value* getValue() const {
        return _value;
    }

    static constexpr const char* Name = "store";

private:
    Value* _pointer;
    Value* _value;
};

class AtomicCompareExchange : public CustomInstruction {
public:
    // XXX.cmpxchg <type> <pointer> <expected> <desired>
    // Where XXX is the instruction set
    explicit AtomicCompareExchange(std::string identifier, Type* type, Value* pointer,
                                  Value* expected, Value* desired) : CustomInstruction(std::move(identifier), {pointer, expected, desired}, type, true), _pointer(pointer), _expected(expected), _desired(desired) {}

    [[nodiscard]] Value* getPointer() const {
        return _pointer;
    }

    [[nodiscard]] Value* getExpected() const {
        return _expected;
    }

    [[nodiscard]] Value* getDesired() const {
        return _desired;
    }

    static constexpr const char* Name = "cmpxchg";

private:
    Value* _pointer;
    Value* _expected;
    Value* _desired;
};

void custom_instruction_set(Context& context, Module& module) {
    auto volatileSet = context.createCustomInstructionSet("volatile");
    volatileSet->addInstruction(VolatileLoad::Name, [&](Type* type, std::vector<Value*> operands) -> std::unique_ptr<CustomInstruction> {
        if (operands.size() != 1) {
            throw std::runtime_error("Volatile load instruction requires exactly one operand");
        }
        return std::make_unique<VolatileLoad>(volatileSet->getName() + "." + VolatileLoad::Name, type, operands[0]);
    });

    volatileSet->addInstruction(VolatileStore::Name, [&](Type* type, std::vector<Value*> operands) -> std::unique_ptr<CustomInstruction> {
        if (operands.size() != 2) {
            throw std::runtime_error("Volatile store instruction requires exactly two operands");
        }
        return std::make_unique<VolatileStore>(volatileSet->getName() + "." + VolatileStore::Name, type, operands[0], operands[1]);
    });

    auto loadInstr = volatileSet->getInstruction(VolatileLoad::Name);
    auto storeInstr = volatileSet->getInstruction(VolatileStore::Name);

    auto atomicSet = context.createCustomInstructionSet("atomic");
    atomicSet->addInstruction(AtomicCompareExchange::Name, [&](Type* type, std::vector<Value*> operands) -> std::unique_ptr<CustomInstruction> {
        if (operands.size() != 3) {
            throw std::runtime_error("Atomic compare exchange instruction requires exactly three operands");
        }
        return std::make_unique<AtomicCompareExchange>(atomicSet->getName() + "." + AtomicCompareExchange::Name, type, operands[0], operands[1], operands[2]);
    });

    auto cmpxchgInstr = atomicSet->getInstruction(AtomicCompareExchange::Name);

    // Where arg0 is a pointer to an i32.
    auto funcType = context.functionType(context.i32(), {context.ptr()}, FunctionType::CallingConvention::C, false);
    auto func = module.addFunction(funcType, "customInstructionSet");

    auto block = func->addEntryBlock();
    auto arg0 = func->getArguments()[0].get();
    auto volatileLoad = block->addCustom(loadInstr(context.i32(), {arg0}));

    auto allocated = block->addAllocate(context.i32());
    block->addCustom(storeInstr(context.i32(), {allocated, volatileLoad}));

    auto expected = context.literal(context.i32(), 42);
    auto desired = context.literal(context.i32(), 55);
    auto cmpxchg = block->addCustom(cmpxchgInstr(context.i32(), {allocated, expected, desired}));

    auto true_block = func->addBlock("true_block");
    auto false_block = func->addBlock("false_block");

    auto condition = block->addEquality(EqualityOperation::Opcode::Eq, EqualityOperation::Signedness::Signed, cmpxchg, expected);
    block->addJumpNotZero(condition, true_block, false_block);

    true_block->addReturn(context.literal(context.i32(), 1));
    false_block->addReturn(false_block->addLoad(context.i32(), allocated));
}

int main(int argc, char* argv[]) {
    Context context;
    Module module(&context, Module::Architecture::Amd64, "test_module");

    //basic(context, module);
    //vaarg(context, module);
    custom_instruction_set(context, module);

    // Test the IREmitter by printing the module to stdout
    IREmitter emitter(std::cout);
    emitter.emitModule(module);

    return 0;
}
