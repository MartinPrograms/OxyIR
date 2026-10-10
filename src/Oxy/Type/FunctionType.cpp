#include "FunctionType.h"

namespace Oxy {
    FunctionType::FunctionType(Type* returnType, std::vector<Type*> paramTypes, bool isVarArg)
        : Type(Kind::Function), _returnType(returnType), _paramTypes(std::move(paramTypes)), _isVarArg(isVarArg) {}

    Type* FunctionType::getReturnType() const {
        return _returnType;
    }

    const std::vector<Type*>& FunctionType::getParamTypes() const {
        return _paramTypes;
    }

    bool FunctionType::isVarArg() const {
        return _isVarArg;
    }
} // Oxy