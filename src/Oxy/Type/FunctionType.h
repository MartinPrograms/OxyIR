#ifndef OXYIR_FUNCTIONTYPE_H
#define OXYIR_FUNCTIONTYPE_H

#include "Type.h"
#include <vector>
#include <memory>

namespace Oxy {
    class FunctionType : public Type {
    public:
        // Inspired from https://llvm.org/doxygen/namespacellvm_1_1CallingConv.html
        enum class CallingConvention { C, Fast, Cold, SysV64, Win64, Aapcs64 };

        explicit FunctionType(Type* returnType, std::vector<Type*> paramTypes, CallingConvention callConv, bool isVarArg = false);
        ~FunctionType() override = default;

        [[nodiscard]] Type* getReturnType() const;
        [[nodiscard]] const std::vector<Type*>& getParamTypes() const;
        [[nodiscard]] CallingConvention getCallingConvention() const;
        [[nodiscard]] bool isVarArg() const;
        [[nodiscard]] bool matches(const Type *other) const override;

    private:
        Type* _returnType;
        std::vector<Type*> _paramTypes;
        CallingConvention _callConv;
        bool _isVarArg;
    };
} // Oxy

#endif //OXYIR_FUNCTIONTYPE_H