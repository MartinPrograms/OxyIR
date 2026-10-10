#include "Context.h"

namespace Oxy {
    Context::Context(DiagnosticHandler handler) : _diagnosticHandler(std::move(handler)) {
        _ptrType = std::make_unique<PointerType>();
        _voidType = std::make_unique<VoidType>();
        _vaListType = std::make_unique<VariadicListType>();

        _primitiveTypes[0] = std::make_unique<PrimitiveType>(PrimitiveType::Kind::I1);
        _primitiveTypes[1] = std::make_unique<PrimitiveType>(PrimitiveType::Kind::I8);
        _primitiveTypes[2] = std::make_unique<PrimitiveType>(PrimitiveType::Kind::I16);
        _primitiveTypes[3] = std::make_unique<PrimitiveType>(PrimitiveType::Kind::I32);
        _primitiveTypes[4] = std::make_unique<PrimitiveType>(PrimitiveType::Kind::I64);
        _primitiveTypes[5] = std::make_unique<PrimitiveType>(PrimitiveType::Kind::F32);
        _primitiveTypes[6] = std::make_unique<PrimitiveType>(PrimitiveType::Kind::F64);

        for (int i = 0; i < 7; ++i) {
            _primitives[i] = _primitiveTypes[i].get();
        }

        if (!_diagnosticHandler) {
            _diagnosticHandler = [](const Diagnostic& diag) {
                std::string severityStr;
                switch (diag.severity) {
                    case Severity::Info: severityStr = "Info"; break;
                    case Severity::Warning: severityStr = "Warning"; break;
                    case Severity::Error: severityStr = "Error"; break;
                }

                std::ostream &out = (diag.severity == Severity::Error) ? std::cerr : std::cout;
                out << severityStr << ": " << diag.message;
                if (!diag.location.file.empty()) {
                    out << " (" << diag.location.file << ":" << diag.location.line << ":" << diag.location.column << ")";
                }
                out << std::endl;
            };
        }
    }

    PointerType* Context::ptr() {
        return _ptrType.get();
    }

    VoidType* Context::voidType() {
        return _voidType.get();
    }

    VariadicListType * Context::vaListType() {
        return _vaListType.get();
    }

    PrimitiveType* Context::i1() { return _primitives[0]; }
    PrimitiveType* Context::i8() { return _primitives[1]; }
    PrimitiveType* Context::i16() { return _primitives[2]; }
    PrimitiveType* Context::i32() { return _primitives[3]; }
    PrimitiveType* Context::i64() { return _primitives[4]; }
    PrimitiveType* Context::f32() { return _primitives[5]; }
    PrimitiveType* Context::f64() { return _primitives[6]; }

    ArrayType* Context::arrayType(Type* elementType, size_t count) {
        auto arrayTy = std::make_unique<ArrayType>(elementType, count);
        ArrayType* ptr = arrayTy.get();
        _arrayTypes.push_back(std::move(arrayTy));
        return ptr;
    }

    StructType* Context::structType(std::vector<Type*> members, size_t alignment) {
        auto structTy = std::make_unique<StructType>(members, alignment);
        StructType* ptr = structTy.get();
        _structTypes.push_back(std::move(structTy));
        return ptr;
    }

    FunctionType* Context::functionType(Type* returnType, std::vector<Type*> params, bool isVarArg) {
        auto funcTy = std::make_unique<FunctionType>(returnType, params, isVarArg);
        FunctionType* ptr = funcTy.get();
        _functionTypes.push_back(std::move(funcTy));
        return ptr;
    }

    Literal* Context::literal(PrimitiveType *i32, Literal::Data data) {
        auto literal = std::make_unique<Literal>(i32, data);
        Literal* ptr = literal.get();
        _literals.push_back(std::move(literal));
        return ptr;
    }

    TypeValue *Context::typeValue(Type *type) {
        auto typeTy = std::make_unique<TypeValue>(type);
        TypeValue* ptr = typeTy.get();
        _typeValues.push_back(std::move(typeTy));
        return ptr;
    }

    CustomInstructionSet * Context::createCustomInstructionSet(std::string name) {
        if (_customInstructionSets.find(name) != _customInstructionSets.end()) {
            throw std::runtime_error("Custom instruction set with name '" + name + "' already exists");
        }

        auto customSet = std::make_unique<CustomInstructionSet>(name);
        CustomInstructionSet* ptr = customSet.get();
        _customInstructionSets.emplace(std::move(name), std::move(customSet));
        return ptr;
    }

    CustomInstructionSet * Context::getCustomInstructionSet(const std::string &name) const {
        auto it = _customInstructionSets.find(name);
        if (it != _customInstructionSets.end()) {
            return it->second.get();
        }
        return nullptr;
    }
} // Oxy