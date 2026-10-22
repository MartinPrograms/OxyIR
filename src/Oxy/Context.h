#ifndef OXYIR_CONTEXT_H
#define OXYIR_CONTEXT_H

#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include <iostream>

#include "CustomInstructionSet.h"
#include "Type/Type.h"
#include "Type/ArrayType.h"
#include "Type/FunctionType.h"
#include "Type/PointerType.h"
#include "Type/PrimitiveType.h"
#include "Type/StructType.h"
#include "Type/VariadicListType.h"
#include "Type/VoidType.h"
#include "Value/Literal.h"
#include "Value/TypeValue.h"

namespace Oxy {
    class Context {
    public:
        enum class Severity { Info, Warning, Error };

        struct SourceLocation {
            std::string file;
            int line;
            int column;
        };

        struct Diagnostic {
            Severity severity;
            std::string message;
            SourceLocation location; // Optional, can be empty if not applicable
        };

        using DiagnosticHandler = std::function<void(const Diagnostic&)>;

        explicit Context(DiagnosticHandler handler = nullptr);
        ~Context() = default;

        PointerType* ptr();
        VoidType* voidType();
        VariadicListType* vaListType();
        PrimitiveType* i1();
        PrimitiveType* i8();
        PrimitiveType* i16();
        PrimitiveType* i32();
        PrimitiveType* i64();
        PrimitiveType* f32();
        PrimitiveType* f64();

        ArrayType* arrayType(Type* elementType, size_t count);
        StructType* structType(std::vector<Type*> members, size_t alignment = 0);
        FunctionType* functionType(Type* returnType, std::vector<Type*> params, FunctionType::CallingConvention callConv, bool isVarArg = false);
        Literal* literal(PrimitiveType* i32, Literal::Data data);
        TypeValue* typeValue(Type* type);


        CustomInstructionSet *createCustomInstructionSet(std::string name);
        [[nodiscard]] CustomInstructionSet *getCustomInstructionSet(const std::string& name) const;

        static std::vector<uint8_t> toBytes(void* ptr, size_t size) {
            std::vector<uint8_t> bytes(size);
            std::memcpy(bytes.data(), ptr, size);
            return bytes;
        }

        void emitDiagnostic(Severity severity, const std::string& message, const SourceLocation& location = {}) {
            if (_diagnosticHandler) {
                _diagnosticHandler({severity, message, location});
            }
        }

        void emitInfo(const std::string& message, const SourceLocation& location = {}) {
            emitDiagnostic(Severity::Info, message, location);
        }

        void emitWarning(const std::string& message, const SourceLocation& location = {}) {
            emitDiagnostic(Severity::Warning, message, location);
        }

        void emitError(const std::string& message, const SourceLocation& location = {}) {
            emitDiagnostic(Severity::Error, message, location);
        }

    private:
        DiagnosticHandler _diagnosticHandler;

        std::unique_ptr<PointerType> _ptrType;
        std::unique_ptr<VoidType> _voidType;
        std::unique_ptr<VariadicListType> _vaListType;
        PrimitiveType* _primitives[7]{}; // i1, i8, i16, i32, i64, f32, f64
        std::unique_ptr<PrimitiveType> _primitiveTypes[7];

        std::vector<std::unique_ptr<ArrayType>> _arrayTypes;
        std::vector<std::unique_ptr<StructType>> _structTypes;
        std::vector<std::unique_ptr<FunctionType>> _functionTypes;
        std::vector<std::unique_ptr<Literal>> _literals;
        std::vector<std::unique_ptr<TypeValue>> _typeValues;

        std::map<std::string, std::unique_ptr<CustomInstructionSet>> _customInstructionSets;
    };
} // Oxy

#endif //OXYIR_CONTEXT_H