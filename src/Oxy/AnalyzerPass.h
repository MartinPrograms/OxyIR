#ifndef OXYIR_ANALYZERPASS_H
#define OXYIR_ANALYZERPASS_H

#include "Module.h"

// An analyzer pass is a pass that analyzes the IR and produces a different kind of output.
namespace Oxy {
    template<typename T>
    class AnalyzerPass {
    public:
        virtual ~AnalyzerPass() = default;
        virtual std::unique_ptr<T> run(const Module& module) = 0;
    };
}

#endif //OXYIR_ANALYZERPASS_H
