#ifndef OXYIR_MODULEPASS_H
#define OXYIR_MODULEPASS_H

#include "Module.h"

// A ModulePass is a transformation that takes in a module, and produces a new module as output. This is used for optimizations and transformations on the IR. A ModulePass can be used to implement things like inlining, dead code elimination, etc.
namespace Oxy {
    class ModulePass {
    public:
        virtual ~ModulePass() = default;
        virtual std::unique_ptr<Module> run(std::unique_ptr<Module> module) = 0;
    };
}

#endif //OXYIR_MODULEPASS_H
