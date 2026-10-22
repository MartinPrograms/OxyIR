#ifndef OXYIR_IRPARSER_H
#define OXYIR_IRPARSER_H

// Takes in text, converts it to a module. This is the opposite of the IREmitter. The IRParser should be able to parse the same IR that the IREmitter emits.

#include <memory>
#include <vector>
#include <string>

#include "Module.h"
#include "Context.h"
#include "Tokenizer.h"

namespace Oxy {
    class IRParser {
    public:
        explicit IRParser(Context* context, std::string input, Module::Architecture arch, std::string name) : _context(context), _input(std::move(input)), _moduleArch(arch), _moduleName(std::move(name)) {}
        ~IRParser() = default;

        std::unique_ptr<Module> parse();

    private:
        Context* _context;
        std::string _input;
        Module::Architecture _moduleArch;
        std::string _moduleName;
        std::unique_ptr<Module> _module;

        std::vector<Token> _tokens;
        size_t _currentTokenIndex = 0;
        std::map<std::string, Type*> _structDefinitions;
        std::map<std::string, Value*> _functionArguments;
        std::map<std::string, Value*> _variables; // locals
        std::map<std::string, BasicBlock*> _blocks;

        Value *parseCustomArgument();
        Value *parseOperand(Type *type);
        Value *parseCustomInstruction(const std::string & table, const std::string & instr, BasicBlock * basic_block);
        Value *parseBinaryInstruction(BasicBlock * basic_block, const std::string & op_name, bool is_signed);
        Value *parseUnaryInstruction(BasicBlock * basic_block, const std::string & string, bool is_signed);
        Value *parseStoreInstruction(BasicBlock * basic_block, const std::string & string);
        Value *parseAllocaInstruction(BasicBlock * basic_block, const std::string & string);
        Value *parseGetElementPointerInstruction(BasicBlock * basic_block, const std::string & string);
        Value *parseCastInstruction(BasicBlock * basic_block, const std::string & op_name, bool is_signed);
        BasicBlock *parseBlockName();
        Value *parseJumpNotZeroInstruction(BasicBlock * basic_block);
        Value *parsePhiInstruction(BasicBlock * basic_block);
        Value *parseCallInstruction(BasicBlock * basic_block);
        Value *parseVaCopyInstruction(BasicBlock * basic_block);
        Value *parseVaArgInstruction(BasicBlock * basic_block);

        Value *parseStatement(BasicBlock *basic_block);
        void parseBody(Function* function);
        void parseFn();
        void parseData();
        void parseStruct();

        std::vector<uint8_t> parseInitializer(Token *token, Type *type);

        Type *parsePrimitiveType(const std::string& name); // i1 - i64, f32, f64
        Type *parseArrayType(); // [type, count]
        Type *parseStructType(); // { type, type, ... }
        Type *parseFunctionType(); // (type, type) -> type or (type, type) conv -> type
        Type *parseType();

        void parseTokens();
        Token *peekToken(size_t offset = 0);
        Token *getToken();

        bool tryConvention(const std::string& value, FunctionType::CallingConvention *out);
        bool verifyCallArguments(FunctionType *fnType, const std::vector<Value *> & vector);
    };
} // Oxy

#endif //OXYIR_IRPARSER_H