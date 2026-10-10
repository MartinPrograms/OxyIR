#include "Module.h"

namespace Oxy {
    Module::Module(Context* context, Architecture arch, std::string name) {
        _context = context;
        _arch = arch;
        _name = std::move(name);
    }

    Context* Module::getContext() const {
        return _context;
    }

    Module::Architecture Module::getArchitecture() const {
        return _arch;
    }

    const std::string &Module::getName() const {
        return _name;
    }

    const std::vector<std::unique_ptr<Function>> &Module::getFunctions() const {
        return _functions;
    }

    const std::vector<std::unique_ptr<Data>> &Module::getGlobalVariables() const {
        return _data;
    }
} // Oxy