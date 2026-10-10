#ifndef OXYIR_IREMITTER_H
#define OXYIR_IREMITTER_H

#include <ostream>

#include "Module.h"

namespace Oxy {
    class IREmitter {
    public:
        explicit IREmitter(std::ostream& output) : _output(output) {}

        void emitModule(const Module& module);

    private:
        void emitFunction(const Function* func);
        void emitBasicBlock(const BasicBlock* block);
        void emitInstruction(const Instruction* instr);
        void emitType(const Type* type);
        void emitValue(const Value* value);
        void emitGlobal(const Data* data);

        std::ostream& _output;
        Context *_context;
        std::map<const Value*, std::string> _valueNames;
        size_t _valueCounter = 0;
        size_t _localValueCounter = 0;

        std::string getNextLocalValueName() {
            return "%" + std::to_string(_localValueCounter++);
        }

        std::string getNextGlobalValueName() {
            return "$" + std::to_string(_valueCounter++);
        }
    };
} // Oxy

#endif //OXYIR_IREMITTER_H