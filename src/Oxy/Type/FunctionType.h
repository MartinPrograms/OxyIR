#ifndef OXYIR_FUNCTIONTYPE_H
#define OXYIR_FUNCTIONTYPE_H

#include "Type.h"
#include <vector>
#include <memory>

namespace Oxy {
    class FunctionType : public Type {
    public:
        explicit FunctionType(Type* returnType, std::vector<Type*> paramTypes, bool isVarArg = false);
        ~FunctionType() override = default;

        [[nodiscard]] Type* getReturnType() const;
        [[nodiscard]] const std::vector<Type*>& getParamTypes() const;
        [[nodiscard]] bool isVarArg() const;

    private:
        Type* _returnType;
        std::vector<Type*> _paramTypes;
        bool _isVarArg;
    };
} // Oxy

#endif //OXYIR_FUNCTIONTYPE_H