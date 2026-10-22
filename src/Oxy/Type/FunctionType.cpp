#include "FunctionType.h"

namespace Oxy {
    FunctionType::FunctionType(Type* returnType, std::vector<Type*> paramTypes, CallingConvention callConv, bool isVarArg)
        : Type(Kind::Function), _returnType(returnType), _paramTypes(std::move(paramTypes)), _callConv(callConv), _isVarArg(isVarArg) {}

    Type* FunctionType::getReturnType() const {
        return _returnType;
    }

    const std::vector<Type*>& FunctionType::getParamTypes() const {
        return _paramTypes;
    }

    FunctionType::CallingConvention FunctionType::getCallingConvention() const {
        return _callConv;
    }

    bool FunctionType::isVarArg() const {
        return _isVarArg;
    }

    bool FunctionType::matches(const Type *other) const {
        auto fnOther = dynamic_cast<const FunctionType*>(other);
        if (!fnOther) {
            return false;
        }

        if (!_returnType->matches(fnOther->_returnType)) {
            return false;
        }

        if (_isVarArg != fnOther->_isVarArg) {
            return false;
        }

        if (_callConv != fnOther->_callConv) {
            return false;
        }

        if (_paramTypes.size() != fnOther->_paramTypes.size()) {
            return false;
        }

        for (size_t i = 0; i < _paramTypes.size(); ++i) {
            if (!_paramTypes[i]->matches(fnOther->_paramTypes[i])) {
                return false;
            }
        }

        return true;
    }
} // Oxy