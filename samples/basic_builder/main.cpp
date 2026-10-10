#include <Oxy/Module.h>

#include "Oxy/IREmitter.h"
#include "Oxy/Architecture/X86_64/X86_64Module.h"

int main() {
    Oxy::Context context;
    Oxy::Module module(&context, Oxy::Module::Architecture::Amd64, "test_module");

    auto mainFunc = module.addFunction(context.functionType(context.i32(), {context.i32(), context.ptr()}), "main", Oxy::Function::Flags::Export);

    auto entryBlock = mainFunc->addEntryBlock();
    auto condBlock = mainFunc->addBlock("cond");
    auto iteratorBlock = mainFunc->addBlock("iterator");
    auto exitBlock = mainFunc->addBlock("exit");

    auto onehundred = context.literal(context.i32(), 100);
    auto i = context.literal(context.i32(), 0);

    // Entry:
    auto iterator = entryBlock->addAllocate(context.i32());
    entryBlock->addStore(iterator, i);
    entryBlock->addJump(condBlock);

    // Cond:
    auto loaded_iterator = condBlock->addLoad(context.i32(), iterator);
    auto cond = condBlock->addEquality(Oxy::Instructions::EqualityOperation::Opcode::Lt, Oxy::Instructions::EqualityOperation::Signedness::Signed, loaded_iterator, onehundred);
    condBlock->addJumpNotZero(cond, iteratorBlock, exitBlock);

    // Iterator:
    auto nextIterator = iteratorBlock->addBinary(Oxy::Instructions::BinaryOperation::Opcode::Add, Oxy::Instructions::BinaryOperation::Signedness::Signed, loaded_iterator, context.literal(context.i32(), 1));
    iteratorBlock->addStore(iterator, nextIterator);
    iteratorBlock->addJump(condBlock);

    // Exit:
    exitBlock->addReturn(context.literal(context.i32(), 0));

    Oxy::IREmitter emitter(std::cout);
    emitter.emitModule(module);

    std::cout << "\n\nLowering to x86-64 machine code...\n\n";

    Oxy::Architecture::X86_64Module x86_64_module(&module);
    x86_64_module.lower();

    return 0;
}
