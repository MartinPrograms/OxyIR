#include "Function.h"

#include "ArgumentValue.h"

namespace Oxy {
    Function::Function(Context *context, FunctionType* type, std::string name, uint32_t flags) : Value(), _context(context), _type(type), _name(std::move(name)), _flags(flags) {
        for (size_t i = 0; i < type->getParamTypes().size(); i++) {
            _arguments.push_back(std::make_unique<ArgumentValue>(type->getParamTypes()[i], i));
        }
    }

    Type* Function::getType() const {
        return _type;
    }

    const std::string& Function::getName() const {
        return _name;
    }

    const std::vector<std::unique_ptr<BasicBlock>> & Function::getBasicBlocks() const {
        return _blocks;
    }

    uint32_t Function::getFlags() const {
        return _flags;
    }

    bool Function::isExtern() const {
        return (_flags & Flags::Extern) != 0;
    }
}
