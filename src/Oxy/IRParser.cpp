#include "IRParser.h"
#include "IREmitter.h"

#include <unordered_set>

namespace Oxy {
    std::unique_ptr<Module> IRParser::parse() {
        _tokens = Tokenizer(_input).tokenize();

        _context->emitInfo("Tokenized input into " + std::to_string(_tokens.size()) + " tokens");
        for (const auto& token : _tokens) {
            _context->emitInfo("Token: " + token.value + " (type: " + std::to_string(static_cast<int>(token.type)) + ", line: " + std::to_string(token.line) + ", column: " + std::to_string(token.column) + ")");
        }

        _module = std::make_unique<Module>(_context, _moduleArch, _moduleName);

        parseTokens();

        // Emit the module again for debugging purposes
        _context->emitInfo("Finished parsing. Module contains " + std::to_string(_module->getFunctions().size()) + " functions and " + std::to_string(_module->getGlobalVariables().size()) + " global variables.");
        IREmitter emitter(std::cout);
        emitter.emitModule(*_module);

        return std::move(_module);
    }

    // For custom instruction tables.
    Value *IRParser::parseCustomArgument() {
        auto token = peekToken();
        if (!token) {
            return nullptr;
        }

        if (token->type == TokenType::Identifier) {
            // This HAS to be a type
            auto type = parseType();
            auto valueToken = peekToken();
            if (valueToken && valueToken->type == TokenType::Syntax && valueToken->value == ",") {
                getToken();
                return _context->typeValue(type);
            }

            if (valueToken && valueToken->type == TokenType::Local) {
                getToken();
                auto varName = valueToken->value;
                if (_variables.find(varName) == _variables.end()) {
                    _context->emitError("Unknown variable '" + varName + "' at line " + std::to_string(valueToken->line) + ", column " + std::to_string(valueToken->column));
                    return nullptr;
                }
                return _variables[varName];
            }

            if (valueToken && valueToken->type == TokenType::Global) {
                getToken();
                auto globalName = valueToken->value;
                const auto& globalVars = _module->getGlobalVariables();

                auto it = std::find_if(globalVars.begin(), globalVars.end(), [&globalName](const std::unique_ptr<Data>& data) {
                    return data->getName() == globalName;
                });

                if (it == globalVars.end()) {
                    // Check if it is a function
                    const auto& functions = _module->getFunctions();
                    auto funcIt = std::find_if(functions.begin(), functions.end(), [&globalName](const std::unique_ptr<Function>& func) {
                        return func->getName() == globalName;
                    });

                    if (funcIt != functions.end()) {
                        return (*funcIt).get();
                    }

                    _context->emitError("Unknown global variable '" + globalName + "' at line " + std::to_string(valueToken->line) + ", column " + std::to_string(valueToken->column));
                    return nullptr;
                }
                return (*it).get();
            }

            if (valueToken && valueToken->type == TokenType::Block) {
                throw new std::runtime_error("im lazy");
            }

            if (valueToken && valueToken->type == TokenType::Number) {
                getToken();
                auto primitiveType = dynamic_cast<PrimitiveType*>(type);
                if (!primitiveType) {
                    _context->emitError("Expected a primitive type for literal at line " + std::to_string(valueToken->line) + ", column " + std::to_string(valueToken->column));
                    return nullptr;
                }

                if (primitiveType->getPrimitiveKind() == PrimitiveType::Kind::F32 || primitiveType->getPrimitiveKind() == PrimitiveType::Kind::F64) {
                    return _context->literal(primitiveType, std::stod(valueToken->value));
                }

                return _context->literal(primitiveType, std::stoll(valueToken->value));
            }

            _context->emitError("Unexpected token '" + token->value + "' when parsing argument at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
            return nullptr;
        }

        _context->emitError("Unexpected token '" + token->value + "' when parsing argument at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
        return nullptr;
    }

    Value *IRParser::parseOperand(Type *type) {
        // Similar to the above but no type parsing.
        auto token = peekToken();
        if (!token) {
            return nullptr;
        }

        if (token->type == TokenType::Local) {
            getToken();
            auto varName = token->value;
            if (_variables.find(varName) == _variables.end()) {
                _context->emitError("Unknown variable '" + varName + "' at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                return nullptr;
            }
            return _variables[varName];
        }

        if (token->type == TokenType::Global) {
            getToken();
            auto globalName = token->value;
            const auto& globalVars = _module->getGlobalVariables();

            auto it = std::find_if(globalVars.begin(), globalVars.end(), [&globalName](const std::unique_ptr<Data>& data) {
                return data->getName() == globalName;
            });

            if (it == globalVars.end()) {
                // Check if it is a function
                const auto& functions = _module->getFunctions();
                auto funcIt = std::find_if(functions.begin(), functions.end(), [&globalName](const std::unique_ptr<Function>& func) {
                    return func->getName() == globalName;
                });

                if (funcIt != functions.end()) {
                    return (*funcIt).get();
                }

                _context->emitError("Unknown global variable '" + globalName + "' at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                return nullptr;
            }
            return (*it).get();
        }

        if (token->type == TokenType::Number) {
            getToken();
            auto primitiveType = dynamic_cast<PrimitiveType*>(type);
            if (!primitiveType) {
                _context->emitError("Expected a primitive type for literal at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                return nullptr;
            }

            if (primitiveType->getPrimitiveKind() == PrimitiveType::Kind::F32 || primitiveType->getPrimitiveKind() == PrimitiveType::Kind::F64) {
                return _context->literal(primitiveType, std::stod(token->value));
            }

            return _context->literal(primitiveType, std::stoll(token->value));
        }

        _context->emitError("Unexpected token '" + token->value + "' when parsing operand at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
        return nullptr;
    }

    Value *IRParser::parseCustomInstruction(const std::string &table, const std::string &instr,
                                            BasicBlock *basic_block) {
        auto customSet = _context->getCustomInstructionSet(table);
        if (!customSet) {
            _context->emitError("Unknown custom instruction set '" + table + "' at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        if (!customSet->hasInstruction(instr)) {
            _context->emitError("Unknown instruction '" + instr + "' in custom instruction set '" + table + "' at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        auto instructionDef = customSet->getInstruction(instr);

        Type* returnType = parseType();
        if (!returnType) {
            _context->emitError("Expected return type for instruction '" + instr + "' in custom instruction set '" + table + "' at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        auto comma = getToken();
        if (!comma && comma->type != TokenType::Syntax && comma->value != ",") {
            _context->emitError("Expected ',' after return type for instruction '" + instr + "' in custom instruction set '" + table + "' at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        // we need to parse the arguments
        std::vector<Value*> arguments;
        size_t currentLine = peekToken()->line;
        while (true) {
            auto token = peekToken();
            if (token && token->line != currentLine) {
                break; // Stop parsing arguments when we reach a new line
            }

            auto arg = parseCustomArgument();
            if (!arg) {
                _context->emitError("Failed to parse argument for instruction '" + instr + "' in custom instruction set '" + table + "' at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return nullptr;
            }

            arguments.push_back(arg);
        }

        auto result = instructionDef(returnType, arguments);
        auto moved = basic_block->addInstruction(std::move(result));

        return moved;
    }

    Value * IRParser::parseBinaryInstruction(BasicBlock *basic_block, const std::string &op_name, bool is_signed) {
        // 'type operand1, operand2'

        auto type = parseType();
        if (!type) {
            return nullptr;
        }

        // We are now at operand1
        auto op1 = parseOperand(type);
        if (!op1) {
            return nullptr;
        }

        auto commaToken = getToken();
        if (!commaToken && commaToken->type != TokenType::Syntax && commaToken->value != ",") {
            return nullptr;
        }

        auto op2 = parseOperand(type);
        if (!op2) {
            return nullptr;
        }

        bool isEquality = false;
        Instructions::BinaryOperation::Opcode opcode = Instructions::BinaryOperation::Opcode::Add;
        if (op_name == "add") {
            opcode = Instructions::BinaryOperation::Opcode::Add;
        } else if (op_name == "sub") {
            opcode = Instructions::BinaryOperation::Opcode::Sub;
        } else if (op_name == "mul") {
            opcode = Instructions::BinaryOperation::Opcode::Mul;
        } else if (op_name == "div") {
            opcode = Instructions::BinaryOperation::Opcode::Div;
        } else if (op_name == "rem") {
            opcode = Instructions::BinaryOperation::Opcode::Rem;
        } else if (op_name == "and") {
            opcode = Instructions::BinaryOperation::Opcode::And;
        } else if (op_name == "or") {
            opcode = Instructions::BinaryOperation::Opcode::Or;
        } else if (op_name == "xor") {
            opcode = Instructions::BinaryOperation::Opcode::Xor;
        } else if (op_name == "shl") {
            opcode = Instructions::BinaryOperation::Opcode::Shl;
        } else if (op_name == "shr") {
            opcode = Instructions::BinaryOperation::Opcode::Shr;
        } else {
            isEquality = true;
        }

        if (!isEquality) {
            Instructions::BinaryOperation::Signedness binarySignedness = is_signed ? Instructions::BinaryOperation::Signedness::Signed : Instructions::BinaryOperation::Signedness::Unsigned;
            if (type->getKind() == Type::Kind::Primitive && (((PrimitiveType*)type)->getPrimitiveKind() == PrimitiveType::Kind::F32 || ((PrimitiveType*)type)->getPrimitiveKind() == PrimitiveType::Kind::F64)) {
                binarySignedness = Instructions::BinaryOperation::Signedness::None; // Floating point operations don't have signedness
            }

            return basic_block->addBinary(opcode, binarySignedness, op1, op2);
        }

        Instructions::EqualityOperation::Opcode eqOpcode;

        if (op_name == "eq") {
            eqOpcode = Instructions::EqualityOperation::Opcode::Eq;
        } else if (op_name == "ne") {
            eqOpcode = Instructions::EqualityOperation::Opcode::Ne;
        } else if (op_name == "lt") {
            eqOpcode = Instructions::EqualityOperation::Opcode::Lt;
        } else if (op_name == "gt") {
            eqOpcode = Instructions::EqualityOperation::Opcode::Gt;
        } else if (op_name == "le") {
            eqOpcode = Instructions::EqualityOperation::Opcode::Le;
        } else if (op_name == "ge") {
            eqOpcode = Instructions::EqualityOperation::Opcode::Ge;
        } else {
            _context->emitError("Unknown binary operation '" + op_name + "' at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        Instructions::EqualityOperation::Signedness equalitySignedness = is_signed ? Instructions::EqualityOperation::Signedness::Signed : Instructions::EqualityOperation::Signedness::Unsigned;
        if (type->getKind() == Type::Kind::Primitive && (((PrimitiveType*)type)->getPrimitiveKind() == PrimitiveType::Kind::F32 || ((PrimitiveType*)type)->getPrimitiveKind() == PrimitiveType::Kind::F64)) {
            equalitySignedness = Instructions::EqualityOperation::Signedness::None; // Floating point comparisons don't have signedness
        }

        return basic_block->addEquality(eqOpcode, equalitySignedness, op1, op2);
    }

    Value *IRParser::parseUnaryInstruction(BasicBlock *basic_block, const std::string &op_name, bool is_signed) {
        auto type = parseType();
        if (!type) {
            return nullptr;
        }

        if (op_name == "ret") {
            if (type->getKind() == Type::Kind::Void) {
                return basic_block->addReturn();
            }
            else {
                if (auto retOp = parseOperand(type))
                    return basic_block->addReturn(retOp);
                else {
                    _context->emitError("Failed to parse return operand for 'ret' instruction at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                    return nullptr;
                }
            }
        }

        auto operand = parseOperand(type);
        if (!operand) {
            return nullptr;
        }

        if (op_name == "neg" || op_name == "not") {
            Instructions::UnaryOperation::Opcode opcode = (op_name == "neg") ? Instructions::UnaryOperation::Opcode::Neg : Instructions::UnaryOperation::Opcode::Not;
            return basic_block->addUnary(opcode, operand);
        }

        if (op_name == "load") {
            return basic_block->addLoad(type, operand);
        }

        if (op_name == "copy") {
            return basic_block->addCopy(operand);
        }

        if (op_name == "bitcast") {
            return basic_block->addBitCast(operand, type);
        }

        _context->emitError("Unknown unary operation '" + op_name + "' at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
        return nullptr;
    }

    Value *IRParser::parseStoreInstruction(BasicBlock *basic_block, const std::string &string) {
        // store type destination, source
        auto type = parseType();

        auto dest = parseOperand(type);
        if (!dest) {
            return nullptr;
        }

        auto commaToken = getToken();
        if (!commaToken && commaToken->type != TokenType::Syntax && commaToken->value != ",") {
            return nullptr;
        }

        auto src = parseOperand(type);
        if (!src) {
            return nullptr;
        }

        return basic_block->addStore(dest, src);
    }

    Value *IRParser::parseAllocaInstruction(BasicBlock *basic_block, const std::string &string) {
        size_t line = peekToken()->line;

        auto type = parseType();
        if (string == "alloca") {
            // alloca type [, count]
            Value* count = nullptr;
            if (peekToken()->line == line) {
                getToken(); // consume the comma
                count = parseOperand(_context->i64());
            }
            else {
                count = _context->literal(_context->i64(), 1);
            }
            return basic_block->addAllocate(type, count);
        }

        // alloca_align type, alignment [, count]
        auto commaToken = getToken();
        if (!commaToken && commaToken->type != TokenType::Syntax && commaToken->value != ",") {
            return nullptr;
        }

        auto alignValue = parseOperand(_context->i64());
        if (!alignValue) {
            return nullptr;
        }

        Value* count = nullptr;
        if (peekToken()->line == line) {
            getToken(); // consume the comma
            count = parseOperand(_context->i64());
        }
        else {
            count = _context->literal(_context->i64(), 1);
        }

        auto alignLiteral = dynamic_cast<Literal*>(alignValue);
        if (!alignLiteral) {
            _context->emitError("Alignment value for 'alloca_align' must be a literal at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        // alignLiteral.getData is a variant of int64_t and double, but it has to be an int64_t for alignment
        if (!std::holds_alternative<int64_t>(alignLiteral->getData())) {
            _context->emitError("Alignment value for 'alloca_align' must be an integer literal at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        size_t alignment = std::get<int64_t>(alignLiteral->getData());
        return basic_block->addAllocate(type, count, alignment);
    }

    Value *IRParser::parseGetElementPointerInstruction(BasicBlock *basic_block, const std::string &string) {
        // gep type base, index0, index1, ...
        auto type = parseType();
        auto base = parseOperand(type);
        if (!base) {
            return nullptr;
        }

        std::vector<Value*> indices;
        size_t line = peekToken()->line;
        while (peekToken() && peekToken()->line == line) {
            auto commaToken = getToken();
            if (!commaToken && commaToken->type != TokenType::Syntax && commaToken->value != ",") {
                return nullptr;
            }
            auto index = parseOperand(_context->i64());
            if (!index) {
                return nullptr;
            }
            indices.push_back(index);
        }

        return basic_block->addGetElementPointer(type, base, indices);
    }

    Value *IRParser::parseCastInstruction(BasicBlock *basic_block, const std::string &op_name, bool is_signed) {
        bool bitCast = false;
        Instructions::CastOperation::Opcode opcode;
        if (op_name == "trunc") {
            opcode = Instructions::CastOperation::Opcode::Trunc;
        } else if (op_name == "zext") {
            opcode = Instructions::CastOperation::Opcode::ZExt;
        } else if (op_name == "sext") {
            opcode = Instructions::CastOperation::Opcode::SExt;
        } else if (op_name == "fptrunc") {
            opcode = Instructions::CastOperation::Opcode::FpTrunc;
        } else if (op_name == "fpext") {
            opcode = Instructions::CastOperation::Opcode::FpExt;
        } else if (op_name == "ptrtoint") {
            opcode = Instructions::CastOperation::Opcode::PtrToInt;
        } else if (op_name == "inttoptr") {
            opcode = Instructions::CastOperation::Opcode::IntToPtr;
        } else if (op_name == "inttofloat") {
            opcode = Instructions::CastOperation::Opcode::IntToFloat;
        } else if (op_name == "floattoint") {
            opcode = Instructions::CastOperation::Opcode::FloatToInt;
        }else if (op_name == "bitcast") {
            bitCast = true;
        } else {
            _context->emitError("Unknown cast operation '" + op_name + "' at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
        }

        auto result_type = parseType();
        if (!result_type) {
            return nullptr;
        }

        auto source_type = parseType();
        if (!source_type) {
            return nullptr;
        }

        auto operand = parseOperand(source_type);
        if (!operand) {
            return nullptr;
        }

        if (bitCast) {
            return basic_block->addBitCast(operand, result_type);
        }

        Instructions::CastOperation::Signedness signedness = is_signed ? Instructions::CastOperation::Signedness::Signed : Instructions::CastOperation::Signedness::Unsigned;
        if (opcode == Instructions::CastOperation::Opcode::FpExt || opcode == Instructions::CastOperation::Opcode::FpTrunc) {
            signedness = Instructions::CastOperation::Signedness::None; // Floating point casts don't have signedness
        }

        return basic_block->addCast(opcode, signedness, operand, result_type);
    }

    BasicBlock *IRParser::parseBlockName() {
        if (!peekToken() || peekToken()->type != TokenType::Block) {
            _context->emitError("Expected block name at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        auto blockName = getToken()->value;
        if (_blocks.find(blockName) == _blocks.end()) {
            _context->emitError("Unknown block name '" + blockName + "' at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        return _blocks[blockName];
    }

    Value *IRParser::parseJumpNotZeroInstruction(BasicBlock *basic_block) {
        auto type = parseType();
        auto condition = parseOperand(type);
        if (!condition) {
            return nullptr;
        }

        auto commaToken = getToken();
        if (!commaToken && commaToken->type != TokenType::Syntax && commaToken->value != ",") {
            return nullptr;
        }

        auto trueBlock = parseBlockName();
        if (!trueBlock) {
            return nullptr;
        }

        commaToken = getToken();
        if (!commaToken && commaToken->type != TokenType::Syntax && commaToken->value != ",") {
            return nullptr;
        }

        auto falseBlock = parseBlockName();
        if (!falseBlock) {
            return nullptr;
        }

        return basic_block->addJumpNotZero(condition, trueBlock, falseBlock);
    }

    Value * IRParser::parsePhiInstruction(BasicBlock *basic_block) {
        // phi type, @label1 operand1, @label2 operand2, ...
        auto type = parseType();
        std::vector<Instructions::PhiOperation::Incoming> incoming;
        size_t line = peekToken()->line;
        while (peekToken() && peekToken()->line == line) {
            auto commaToken = getToken();
            if (!commaToken && commaToken->type != TokenType::Syntax && commaToken->value != ",") {
                return nullptr;
            }

            if (!peekToken() || peekToken()->type != TokenType::Block) {
                _context->emitError("Expected block name in phi instruction at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return nullptr;
            }

            auto blockName = getToken()->value;
            if (_blocks.find(blockName) == _blocks.end()) {
                _context->emitError("Unknown block name '" + blockName + "' in phi instruction at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return nullptr;
            }
            auto block = _blocks[blockName];

            auto operand = parseOperand(type);
            if (!operand) {
                return nullptr;
            }

            incoming.emplace_back(operand, block);
        }

        return basic_block->addPhi(type, incoming);
    }

    Value * IRParser::parseCallInstruction(BasicBlock *basic_block) {
        // call returnType functionTarget, argType1 arg1, argType2 arg2, ...
        auto returnType = parseType();
        if (!returnType) {
            return nullptr;
        }

        // functionName does not have to be a identifier, it can be any value as long as it evaluates to a function pointer.
        auto functionValue = parseOperand(_context->ptr());
        if (!functionValue) {
            return nullptr;
        }

        auto commaToken = getToken();
        if (!commaToken && commaToken->type != TokenType::Syntax && commaToken->value != ",") {
            return nullptr;
        }

        std::vector<Value*> arguments;
        size_t line = peekToken()->line;
        while (peekToken() && peekToken()->line == line) {
            if (peekToken()->type == TokenType::Syntax && peekToken()->value == ",") {
                getToken(); // consume the comma
            }

            if (peekToken() && peekToken()->type == TokenType::Syntax && peekToken()->value == "...") {
                getToken(); // consume the ellipsis
                continue;
            }

            auto argType = parseType();
            if (!argType) {
                return nullptr;
            }

            auto argValue = parseOperand(argType);
            if (!argValue) {
                return nullptr;
            }

            arguments.push_back(argValue);
        }

        return basic_block->addCall(functionValue, arguments);
    }

    Value *IRParser::parseVaCopyInstruction(BasicBlock *basic_block) {
        // vacopy dest, src
        auto dest = parseOperand(_context->ptr());
        if (!dest) {
            return nullptr;
        }

        auto commaToken = getToken();
        if (!commaToken && commaToken->type != TokenType::Syntax && commaToken->value != ",") {
            return nullptr;
        }

        auto src = parseOperand(_context->ptr());
        if (!src) {
            return nullptr;
        }

        return basic_block->addVaCopy(dest, src);
    }

    Value *IRParser::parseStatement(BasicBlock *basic_block) {
        auto opToken = getToken();
        if (!opToken) { return nullptr; }

        std::string opName = opToken->value;
        if (opName.contains('.')) {
            auto table = opName.substr(0, opName.find('.'));
            auto instr = opName.substr(opName.find('.') + 1);

            return parseCustomInstruction(table, instr, basic_block);
        }

        static const std::unordered_set<std::string> signedSuffixableOperations = {
            "add", "sub", "mul", "div", "rem",
            "lt", "gt", "le", "ge",
            "trunc", "zext", "sext", "inttofloat"
        };

        bool isSigned = false;
        char last = opName.back();
        std::string baseOp = opName.substr(0, opName.size() - 1);
        if (last == 's' || last == 'u') {
            if (!signedSuffixableOperations.count(baseOp)) {
                _context->emitError("Operation '" + baseOp + "' cannot be suffixed with 's' or 'u' for signedness at line " + std::to_string(opToken->line) + ", column " + std::to_string(opToken->column));
                return nullptr;
            }
            isSigned = (last == 's');
            opName = baseOp;
        }

        static const std::unordered_set<std::string> binaryOps = {
            "add", "sub", "mul", "div", "rem",
            "and", "or",  "xor", "shl", "shr",
            "eq",  "ne",  "lt",  "gt",  "le",  "ge"
        };

        if (binaryOps.count(opName))
            return parseBinaryInstruction(basic_block, opName, isSigned);


        static const std::unordered_set<std::string> unaryOps = {
            "neg", "not", "load", "copy", "ret"
        };

        if (unaryOps.count(opName))
            return parseUnaryInstruction(basic_block, opName, isSigned);


        static const std::unordered_set<std::string> castOps {
            "trunc", "zext", "sext", "fptrunc", "fpext", "ptrtoint", "inttoptr", "inttofloat", "floattoint", "bitcast"
        };

        if (castOps.count(opName))
            return parseCastInstruction(basic_block, opName, isSigned);

        if (opName == "store") return parseStoreInstruction(basic_block, opName);
        if (opName == "alloca" || opName == "alloca_align") return parseAllocaInstruction(basic_block, opName);
        if (opName == "gep") return parseGetElementPointerInstruction(basic_block, opName);
        if (opName == "jmp") return basic_block->addJump(parseBlockName());
        if (opName == "jnz") return parseJumpNotZeroInstruction(basic_block);
        if (opName == "phi") return parsePhiInstruction(basic_block);
        if (opName == "vastart") return basic_block->addVaStart(parseOperand(_context->ptr()));
        if (opName == "vacopy") return parseVaCopyInstruction(basic_block);
        if (opName == "vaarg") return basic_block->addVaArg(parseType(), parseOperand(_context->ptr()));
        if (opName == "call") return parseCallInstruction(basic_block);

        return nullptr;
    }

    void IRParser::parseBody(Function *function) {
        // We start on a {, but we arent allowed to have nested blocks, so we can just loop until we hit the next one.
        if (!peekToken() || peekToken()->type != TokenType::Syntax || peekToken()->value != "{") {
            _context->emitError("Expected '{' at the beginning of function body at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return;
        }

        getToken();

        _blocks.clear();
        BasicBlock *currentBlock = nullptr;

        size_t currentTokenIndexForBlocks = 0;
        while (true) {
            auto token = peekToken(currentTokenIndexForBlocks);
            if (!token) {
                _context->emitError("Unexpected end of input while looking for blocks in function body at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return;
            }

            if (token->type == TokenType::Syntax && token->value == "}") {
                break; // End of function body, stop looking for blocks
            }

            if (token->type == TokenType::Block) {
                // Make sure this block is the first token on its line
                auto prev = peekToken(currentTokenIndexForBlocks - 1);
                if (prev && prev->line == token->line) {
                    currentTokenIndexForBlocks++; // Probably in a jmp, jnz or phi statement
                    continue;
                }

                auto blockName = token->value;
                if (_blocks.find(blockName) != _blocks.end()) {
                    _context->emitError("Duplicate block name '" + blockName + "' in function body at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                    return;
                }
                auto block = function->addBlock(blockName);
                _blocks[blockName] = block;
            }

            currentTokenIndexForBlocks++;
        }

        while (true) {
            auto token = peekToken();

            if (!token) {
                _context->emitError("Unexpected end of input while parsing function body at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return;
            }

            if (token->type == TokenType::Syntax && token->value == "}") {
                getToken(); // Consume the '}'
                break; // End of function body
            }

            if (token->type == TokenType::Block) {
                currentBlock = _blocks[token->value];
                getToken(); // Consume the block token
                continue;
            }

            // %r = op(signedness) type %arg0 %arg1, %arg2, ..., stuff.
            // or
            // op(signedness) type %arg0 %arg1, %arg2, ..., stuff.

            if (!currentBlock) {
                _context->emitError("Instruction outside of any block at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                return;
            }

            if (token->type == TokenType::Local) {
                // %r = op(signedness) type %arg0 %arg1, %arg2, ..., stuff.
                std::string name = token->value;
                getToken();

                if (peekToken() && peekToken()->type != TokenType::Syntax && peekToken()->value != "=") {
                    _context->emitError("Expected '=' after local variable name at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                    return;
                }

                getToken(); // consume '='

                auto result = parseStatement(currentBlock);
                if (!result) {
                    _context->emitError("Failed to parse instruction for local variable '" + name + "' at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                    return;
                }

                if (_variables.find(name) != _variables.end()) {
                    _context->emitError("Duplicate local variable name '" + name + "' at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                    return;
                }

                _variables[name] = result;

                // Success!
            }
            else {
                // op(signedness) type %arg0 %arg1, %arg2, ..., stuff.
                if (!parseStatement(currentBlock)) {
                    _context->emitError("Failed to parse instruction at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                    return;
                }
            }
        }
    }

    void IRParser::parseFn() {
        _functionArguments.clear();

        // fn <type> <flags> $<name>(<type1> <arg1>, <type2> <arg2>, ...)
        auto fnKeyword = getToken();

        auto returnType = parseType();
        if (!returnType) {
            _context->emitError("Expected return type after 'fn' keyword in function declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return;
        }

        uint32_t flags = 0;
        while (true) {
            auto token = peekToken();
            if (!token) {
                _context->emitError("Unexpected end of input while parsing function declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return;
            }

            if (token->type == TokenType::Identifier) {
                if (token->value == "export") {
                    flags |= (uint32_t)Function::Flags::Export;
                    getToken();
                } else if (token->value == "inline") {
                    flags |= (uint32_t)Function::Flags::Inline;
                    getToken();
                } else if (token->value == "extern") {
                    flags |= (uint32_t)Function::Flags::Extern;
                    getToken();
                } else {
                    _context->emitError("Unknown flag '" + token->value + "' in function declaration at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                    return;
                }
            } else {
                break; // No more flags, move on to the name token
            }
        }

        auto nameToken = getToken();
        if (!nameToken || nameToken->type != TokenType::Global) {
            _context->emitError("Expected global function name after function type in function declaration at line " + std::to_string(nameToken ? nameToken->line : 0) + ", column " + std::to_string(nameToken ? nameToken->column : 0));
            return;
        }

        std::vector<Type*> paramTypes;
        std::vector<std::string> paramNames;
        bool isVarArg = false;

        auto openParenToken = getToken();
        if (!openParenToken || openParenToken->type != TokenType::Syntax || openParenToken->value != "(") {
            _context->emitError("Expected '(' after function name in function declaration at line " + std::to_string(openParenToken ? openParenToken->line : 0) + ", column " + std::to_string(openParenToken ? openParenToken->column : 0));
            return;
        }

        while (true) {
            auto token = peekToken();
            if (!token) {
                _context->emitError("Unexpected end of input while parsing function parameters at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return;
            }

            if (token->type == TokenType::Syntax && token->value == ")") {
                getToken(); // Consume ')'
                break; // End of parameter list
            }

            if (token->type == TokenType::Syntax && token->value == "...") {
                getToken(); // Consume '...'
                isVarArg = true;
                continue; // Varargs must be the last element, so we can ignore any further tokens
            }

            auto paramType = parseType();
            if (!paramType) {
                _context->emitError("Expected parameter type in function declaration at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                return;
            }
            paramTypes.push_back(paramType);

            auto identToken = peekToken();
            if (identToken && identToken->type == TokenType::Local) {
                auto paramName = identToken->value;
                paramNames.push_back(paramName);
                _functionArguments[paramName] = nullptr; // We don't have the actual ArgumentValue yet
                getToken();
            } else {
                _context->emitError("Expected parameter name after parameter type in function declaration at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                return;
            }

            auto nextToken = peekToken();
            if (nextToken && nextToken->type == TokenType::Syntax && nextToken->value == ",") {
                getToken(); // Consume ','
            } else if (nextToken && nextToken->type == TokenType::Syntax && nextToken->value == ")") {
                continue; // Next iteration will consume ')'
            } else {
                _context->emitError("Expected ',' or ')' after parameter type in function declaration at line " + std::to_string(nextToken ? nextToken->line : 0) + ", column " + std::to_string(nextToken ? nextToken->column : 0));
                return;
            }
        }

        auto funcType = _context->functionType(returnType, paramTypes, isVarArg);
        auto func = _module->addFunction(funcType, nameToken->value, flags);
        _variables.clear(); // Clear the variable map for the new function

        for (size_t i = 0; i < paramTypes.size(); i++) {
            auto paramName = paramNames[i];
            _functionArguments[paramName] = func->getArguments()[i].get();
            _variables[paramName] = func->getArguments()[i].get();
        }

        // Check if theres a {, if so we have a function body to parse. If not, this is just a declaration.
        auto nextToken = peekToken();
        if (nextToken && nextToken->type == TokenType::Syntax && nextToken->value == "{") {
            parseBody(func);
        }
    }

    void IRParser::parseData() {
        auto dataKeyword = getToken();

        auto type = parseType();
        if (!type) {
            _context->emitError("Expected type after data declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return;
        }

        uint32_t flags = 0;
        size_t alignment = 0;

        while (true) {
            auto token = peekToken();
            if (!token) {
                _context->emitError("Unexpected end of input while parsing data declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return;
            }

            if (token->type == TokenType::Identifier) {
                if (token->value == "export") {
                    flags |= (uint32_t)Data::Flags::Export;
                    getToken();
                } else if (token->value == "thread_local") {
                    flags |= (uint32_t)Data::Flags::ThreadLocal;
                    getToken();
                } else if (token->value == "mutable") {
                    flags |= (uint32_t)Data::Flags::Mutable;
                    getToken();
                } else if (token->value == "align") {
                    flags |= (uint32_t)Data::Flags::Align;
                    getToken(); // Consume 'align' keyword
                    auto alignValueToken = getToken();
                    if (!alignValueToken || alignValueToken->type != TokenType::Number) {
                        _context->emitError("Expected numeric alignment value after 'align' in data declaration at line " + std::to_string(alignValueToken ? alignValueToken->line : 0) + ", column " + std::to_string(alignValueToken ? alignValueToken->column : 0));
                        return;
                    }
                    try {
                        alignment = std::stoul(alignValueToken->value);
                    } catch (const std::exception& e) {
                        _context->emitError("Invalid alignment value '" + alignValueToken->value + "' at line " + std::to_string(alignValueToken ? alignValueToken->line : 0) + ", column " + std::to_string(alignValueToken ? alignValueToken->column : 0) + ": " + e.what());
                        return;
                    }
                } else {
                    _context->emitError("Unknown flag '" + token->value + "' in data declaration at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                    return;
                }
            } else {
                break; // No more flags, move on to the name token
            }
        }

        auto nameToken = getToken();
        if (!nameToken || nameToken->type != TokenType::Global) {
            _context->emitError("Expected global variable name after data declaration at line " + std::to_string(nameToken ? nameToken->line : 0) + ", column " + std::to_string(nameToken ? nameToken->column : 0));
            return;
        }

        auto equalsToken = getToken();
        if (!equalsToken || equalsToken->type != TokenType::Syntax || equalsToken->value != "=") {
            _context->emitError("Expected '=' after global variable name in data declaration at line " + std::to_string(equalsToken ? equalsToken->line : 0) + ", column " + std::to_string(equalsToken ? equalsToken->column : 0));
            return;
        }

        // there can be 4 kinds of initializers:
        // - a byte array initializer: [0x00, 0x01, ...] (valid for any type, as all initializers are lowered to byte arrays)
        // - a string initializer: "string literal" (valid for [i8, N] types, will be converted to byte array with NO null terminator, unless \0 is explicitly included in the string)
        // - a numeric initializer: 123, 0x1A, 3.14 (valid for primitive types, will be converted to byte array of the appropriate size and endianness)
        // - an array initializer { ... }, which is also a struct initializer

        auto initializerToken = getToken();
        std::vector<uint8_t> initialValue = parseInitializer(initializerToken, type);

        _module->addGlobalVariable(type, nameToken->value, static_cast<Data::Flags>(flags), alignment, initialValue);
    }

    void IRParser::parseStruct() {
        auto structKeyword = getToken();

        // We have potential flags, currently only `align N`
        size_t alignment = 0;
        while (true) {
            auto token = peekToken();
            if (!token) {
                _context->emitError("Unexpected end of input while parsing struct declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return;
            }

            if (token->type == TokenType::Identifier && token->value == "align") {
                getToken(); // Consume 'align' keyword
                auto alignValueToken = getToken();
                if (!alignValueToken || alignValueToken->type != TokenType::Number) {
                    _context->emitError("Expected numeric alignment value after 'align' in struct declaration at line " + std::to_string(alignValueToken ? alignValueToken->line : 0) + ", column " + std::to_string(alignValueToken ? alignValueToken->column : 0));
                    return;
                }
                try {
                    alignment = std::stoul(alignValueToken->value);
                } catch (const std::exception& e) {
                    _context->emitError("Invalid alignment value '" + alignValueToken->value + "' at line " + std::to_string(alignValueToken ? alignValueToken->line : 0) + ", column " + std::to_string(alignValueToken ? alignValueToken->column : 0) + ": " + e.what());
                    return;
                }
            } else {
                break; // No more flags, move on to the name token
            }
        }

        auto nameToken = getToken();
        if (!nameToken || nameToken->type != TokenType::TypeDefinition) {
            _context->emitError("Expected struct name after struct declaration at line " + std::to_string(nameToken ? nameToken->line : 0) + ", column " + std::to_string(nameToken ? nameToken->column : 0));
            return;
        }

        auto openBraceToken = getToken();
        if (!openBraceToken || openBraceToken->type != TokenType::Syntax || openBraceToken->value != "{") {
            _context->emitError("Expected '{' after struct name in struct declaration at line " + std::to_string(openBraceToken ? openBraceToken->line : 0) + ", column " + std::to_string(openBraceToken ? openBraceToken->column : 0));
            return;
        }

        std::vector<Type*> memberTypes;
        while (true) {
            auto nextToken = peekToken();
            if (!nextToken) {
                _context->emitError("Unexpected end of input while parsing struct members at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return;
            }
            if (nextToken->type == TokenType::Syntax && nextToken->value == "}") {
                getToken(); // Consume the '}'
                break;
            }

            auto memberType = parseType();
            if (!memberType) {
                _context->emitError("Expected type for struct member at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return;
            }
            memberTypes.push_back(memberType);

            auto commaOrClose = peekToken();
            if (commaOrClose->type == TokenType::Syntax && commaOrClose->value == ",") {
                getToken(); // Consume the ','
            } else if (commaOrClose->type == TokenType::Syntax && commaOrClose->value == "}") {
                // Will be consumed in the next iteration
            } else {
                _context->emitError("Expected ',' or '}' after struct member type at line " + std::to_string(commaOrClose ? commaOrClose->line : 0) + ", column " + std::to_string(commaOrClose ? commaOrClose->column : 0));
                return;
            }
        }

        _structDefinitions[nameToken->value] = _context->structType(memberTypes, alignment);
    }

    std::vector<uint8_t> IRParser::parseInitializer(Token *token, Type *type) {
        // This is a recursive function that will parse the initializer based on the type.
        if (token->type == TokenType::Syntax && token->value == "[") {
            std::vector<uint8_t> bytes;
            while (true) {
                auto nextToken = peekToken();
                if (nextToken->value == "]") {
                    getToken(); // Consume the ']'
                    break;
                }

                auto byteToken = getToken();
                if (!byteToken || (byteToken->type != TokenType::Number && byteToken->type != TokenType::Number)) {
                    _context->emitError("Expected byte value in array initializer at line " + std::to_string(byteToken ? byteToken->line : 0) + ", column " + std::to_string(byteToken ? byteToken->column : 0));
                    return {};
                }

                try {
                    uint8_t byteValue = static_cast<uint8_t>(std::stoul(byteToken->value, nullptr, 0));
                    bytes.push_back(byteValue);
                } catch (const std::exception& e) {
                    _context->emitError("Invalid byte value '" + byteToken->value + "' in array initializer at line " + std::to_string(byteToken ? byteToken->line : 0) + ", column " + std::to_string(byteToken ? byteToken->column : 0) + ": " + e.what());
                    return {};
                }

                auto commaOrClose = peekToken();
                if (commaOrClose->type == TokenType::Syntax && commaOrClose->value == ",") {
                    getToken(); // Consume the ','
                } else if (commaOrClose->type == TokenType::Syntax && commaOrClose->value == "]") {
                    // Will be consumed in the next iteration
                } else {
                    _context->emitError("Expected ',' or ']' after byte value in array initializer at line " + std::to_string(commaOrClose ? commaOrClose->line : 0) + ", column " + std::to_string(commaOrClose ? commaOrClose->column : 0));
                    return {};
                }
            }
            return bytes;
        }

        if (token->type == TokenType::String) {
            const std::string& str = token->value;
            auto bytes = std::vector<uint8_t>(str.begin(), str.end());

            // Remove any \0 null terminator if present, and replace with actual null
            for (size_t i = 0; i < bytes.size(); i++) {
                if (bytes[i] == '\\' && i + 1 < bytes.size() && bytes[i + 1] == '0') {
                    bytes[i] = '\0';
                    bytes.erase(bytes.begin() + i + 1);
                }
            }

            return bytes;
        }

        if (token->type == TokenType::Number) {
            try {
                if (auto primitiveType = dynamic_cast<PrimitiveType*>(type)) {
                    size_t size = 0;
                    if (primitiveType->getPrimitiveKind() == PrimitiveType::Kind::I1) size = 1;
                    else if (primitiveType->getPrimitiveKind() == PrimitiveType::Kind::I8) size = 1;
                    else if (primitiveType->getPrimitiveKind() == PrimitiveType::Kind::I16) size = 2;
                    else if (primitiveType->getPrimitiveKind() == PrimitiveType::Kind::I32) size = 4;
                    else if (primitiveType->getPrimitiveKind() == PrimitiveType::Kind::I64) size = 8;
                    else if (primitiveType->getPrimitiveKind() == PrimitiveType::Kind::F32) size = 4;
                    else if (primitiveType->getPrimitiveKind() == PrimitiveType::Kind::F64) size = 8;

                    if (primitiveType->getPrimitiveKind() != PrimitiveType::Kind::F32 && primitiveType->getPrimitiveKind() != PrimitiveType::Kind::F64) {
                        int64_t intValue = std::stoll(token->value, nullptr, 0);
                        return Context::toBytes(&intValue, size);
                    }

                    if (primitiveType->getPrimitiveKind() == PrimitiveType::Kind::F32) {
                        float floatValue = std::stof(token->value);
                        return Context::toBytes(&floatValue, size);
                    }

                    double floatValue = std::stod(token->value);
                    return Context::toBytes(&floatValue, size);
                }
                if (dynamic_cast<PointerType*>(type) || dynamic_cast<FunctionType*>(type)) {
                    uint64_t ptrValue = std::stoull(token->value, nullptr, 0);
                    return Context::toBytes(&ptrValue, Module::pointerSizes[_moduleArch]);
                }
                _context->emitError("Numeric initializer is only valid for primitive types at line " + std::to_string(token ? token->line : 0) + ", column " + std::to_string(token ? token->column : 0));
                return {};
            } catch (const std::exception& e) {
                _context->emitError("Invalid numeric value '" + token->value + "' in initializer at line " + std::to_string(token ? token->line : 0) + ", column " + std::to_string(token ? token->column : 0) + ": " + e.what());
                return {};
            }
        }

        if (token->type == TokenType::Syntax && token->value == "{") {
            std::vector<uint8_t> bytes;
            size_t index = 0;
            while (true) {
                auto nextToken = peekToken();
                if (nextToken->type == TokenType::Syntax && nextToken->value == "}") {
                    getToken(); // Consume the '}'
                    break;
                }

                if (type->getKind() == Type::Kind::Array) {
                    Type* innerType = nullptr;
                    auto arrayType = dynamic_cast<ArrayType*>(type);
                    innerType = arrayType->getElementType();

                    auto valueToken = getToken();
                    auto valueBytes = parseInitializer(valueToken, innerType);
                    bytes.insert(bytes.end(), valueBytes.begin(), valueBytes.end());
                }
                else if (type->getKind() == Type::Kind::Struct) {
                    auto structType = dynamic_cast<StructType*>(type);
                    if (structType->getMemberTypes().empty()) {
                        _context->emitError("Struct type with no members cannot be initialized with a struct initializer at line " + std::to_string(token ? token->line : 0) + ", column " + std::to_string(token ? token->column : 0));
                        return {};
                    }
                    Type* memberType = structType->getMemberTypes()[index];
                    index++;

                    auto valueToken = getToken();
                    auto valueBytes = parseInitializer(valueToken, memberType);
                    bytes.insert(bytes.end(), valueBytes.begin(), valueBytes.end());
                } else {
                    _context->emitError("Struct initializer is only valid for struct types at line " + std::to_string(token ? token->line : 0) + ", column " + std::to_string(token ? token->column : 0));
                    return {};
                }

                auto commaOrClose = peekToken();
                if (commaOrClose->type == TokenType::Syntax && commaOrClose->value == ",") {
                    getToken(); // Consume the ','
                } else if (commaOrClose->type == TokenType::Syntax && commaOrClose->value == "}") {
                    // Will be consumed in the next iteration
                } else {
                    _context->emitError("Expected ',' or '}' after value in struct initializer at line " + std::to_string(commaOrClose ? commaOrClose->line : 0) + ", column " + std::to_string(commaOrClose ? commaOrClose->column : 0));
                    return {};
                }
            }
            return bytes;
        }

        _context->emitError("Invalid initializer token '" + token->value + "' at line " + std::to_string(token ? token->line : 0) + ", column " + std::to_string(token ? token->column : 0));
        return {};
    }

    Type* IRParser::parsePrimitiveType(const std::string &name) {
        getToken(); // Consume the type name token
        if (name == "i1") return _context->i1();
        if (name == "i8") return _context->i8();
        if (name == "i16") return _context->i16();
        if (name == "i32") return _context->i32();
        if (name == "i64") return _context->i64();
        if (name == "f32") return _context->f32();
        if (name == "f64") return _context->f64();
        if (name == "void") return _context->voidType();
        if (name == "va_list") return _context->vaListType();
        _context->emitError("Unknown primitive type '" + name + "' at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
        return nullptr;
    }

    Type* IRParser::parseArrayType() {
        auto openBracket = getToken();
        auto elementType = parseType();
        if (!elementType) {
            _context->emitError("Expected element type in array type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        auto comma = getToken();
        if (!comma || comma->type != TokenType::Syntax || comma->value != ",") {
            _context->emitError("Expected ',' after element type in array type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        auto countToken = getToken();
        if (!countToken || countToken->type != TokenType::Number) {
            _context->emitError("Expected array size after ',' in array type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        auto closeBracket = getToken();
        if (!closeBracket || closeBracket->type != TokenType::Syntax || closeBracket->value != "]") {
            _context->emitError("Expected ']' at the end of array type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        size_t count = std::stoul(countToken->value);
        return _context->arrayType(elementType, count);
    }

    Type* IRParser::parseStructType() {
        // Anonymous struct type, defined as `struct <flags> { type, type, ... }`
        // The only difference is that this skips the name token, and directly parses the type and members. Internally will resolve back to this, as named types are lost after parsing.
        auto structKeyword = getToken();
        size_t alignment = 0;
        while (true) {
            auto token = peekToken();
            if (!token) {
                _context->emitError("Unexpected end of input while parsing struct type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return nullptr;
            }

            if (token->type == TokenType::Identifier && token->value == "align") {
                getToken(); // Consume 'align' keyword
                auto alignValueToken = getToken();
                if (!alignValueToken || alignValueToken->type != TokenType::Number) {
                    _context->emitError("Expected numeric alignment value after 'align' in struct type declaration at line " + std::to_string(alignValueToken ? alignValueToken->line : 0) + ", column " + std::to_string(alignValueToken ? alignValueToken->column : 0));
                    return nullptr;
                }
                try {
                    alignment = std::stoul(alignValueToken->value);
                } catch (const std::exception& e) {
                    _context->emitError("Invalid alignment value '" + alignValueToken->value + "' in struct type declaration at line " + std::to_string(alignValueToken ? alignValueToken->line : 0) + ", column " + std::to_string(alignValueToken ? alignValueToken->column : 0) + ": " + e.what());
                    return nullptr;
                }
            } else {
                break; // No more flags, move on to the members
            }
        }

        auto openBraceToken = getToken();
        if (!openBraceToken || openBraceToken->type != TokenType::Syntax || openBraceToken->value != "{") {
            _context->emitError("Expected '{' after 'struct' keyword in struct type declaration at line " + std::to_string(openBraceToken ? openBraceToken->line : 0) + ", column " + std::to_string(openBraceToken ? openBraceToken->column : 0));
            return nullptr;
        }

        std::vector<Type*> memberTypes;
        while (true) {
            auto nextToken = peekToken();
            if (!nextToken) {
                _context->emitError("Unexpected end of input while parsing struct type members at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return nullptr;
            }

            if (nextToken->type == TokenType::Syntax && nextToken->value == "}") {
                getToken(); // Consume the '}'
                break;
            }

            auto memberType = parseType();
            if (!memberType) {
                _context->emitError("Expected type for struct member in struct type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return nullptr;
            }
            memberTypes.push_back(memberType);

            auto commaOrClose = peekToken();
            if (commaOrClose->type == TokenType::Syntax && commaOrClose->value == ",") {
                getToken(); // Consume the ','
            } else if (commaOrClose->type == TokenType::Syntax && commaOrClose->value == "}") {
                // Will be consumed in the next iteration
            } else {
                _context->emitError("Expected ',' or '}' after struct member type in struct type declaration at line " + std::to_string(commaOrClose ? commaOrClose->line : 0) + ", column " + std::to_string(commaOrClose ? commaOrClose->column : 0));
                return nullptr;
            }
        }

        return _context->structType(memberTypes, alignment);
    }

    Type* IRParser::parseFunctionType() {
        auto openParen = getToken();
        std::vector<Type*> paramTypes;
        bool isVarArg = false;
        while (true) {
            auto nextToken = peekToken();
            if (!nextToken) {
                _context->emitError("Unexpected end of input while parsing function type parameters");
                return nullptr;
            }

            if (nextToken->type == TokenType::Syntax && nextToken->value == ")") {
                getToken(); // Consume the ')'
                break;
            }

            if (nextToken->type == TokenType::Syntax && nextToken->value == "...") {
                getToken(); // Consume the '...'
                isVarArg = true;
                auto closeParen = peekToken();
                if (!closeParen || closeParen->type != TokenType::Syntax || closeParen->value != ")") {
                    _context->emitError("Expected ')' after '...' in function type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                    return nullptr;
                }
                continue;
            }

            auto paramType = parseType();
            if (!paramType) {
                _context->emitError("Expected parameter type in function type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return nullptr;
            }
            paramTypes.push_back(paramType);

            auto commaOrClose = peekToken();
            if (commaOrClose->type == TokenType::Syntax && commaOrClose->value == ",") {
                getToken(); // Consume the ','
            } else if (commaOrClose->type == TokenType::Syntax && commaOrClose->value == ")") {
                // Will be consumed in the next iteration
            } else {
                _context->emitError("Expected ',' or ')' after parameter type in function type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
                return nullptr;
            }
        }

        auto arrow = getToken();
        if (!arrow || arrow->type != TokenType::Syntax || arrow->value != "->") {
            _context->emitError("Expected '->' after parameter list in function type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        auto returnType = parseType();
        if (!returnType) {
            _context->emitError("Expected return type in function type declaration at line " + std::to_string(peekToken() ? peekToken()->line : 0) + ", column " + std::to_string(peekToken() ? peekToken()->column : 0));
            return nullptr;
        }

        return _context->functionType(returnType, paramTypes, isVarArg);
    }

    Type* IRParser::parseType() {
        auto token = peekToken();

        if (token->value == "ptr") {
            getToken(); // Consume 'ptr'
            return _context->ptr();
        }

        if (token->type == TokenType::Identifier) {
            return parsePrimitiveType(token->value);
        }

        if (token->type == TokenType::Syntax && token->value == "[") {
            return parseArrayType();
        }

        if (token->type == TokenType::TypeDefinition) {
            if (_structDefinitions.find(token->value) == _structDefinitions.end()) {
                _context->emitError("Unknown struct type '" + token->value + "' at line " + std::to_string(token->line) + ", column " + std::to_string(token->column));
                return nullptr;
            }

            getToken(); // Consume the struct type name
            return _structDefinitions[token->value];
        }

        if (token->type == TokenType::Keyword && token->value == "struct") {
            return parseStructType();
        }

        if (token->type == TokenType::Syntax && token->value == "(") {
            return parseFunctionType();
        }

        _context->emitError("Unexpected token '" + token->value + "' at line " + std::to_string(token->line) + ", column " + std::to_string(token->column) + " while parsing type");
        return nullptr;
    }

    void IRParser::parseTokens() {
        // We only have 3 valid top level statements, fn, data, and struct
        if (_tokens.empty()) {
            _context->emitError("No tokens to parse");
            return;
        }

        while (peekToken()) {
            auto token = peekToken();
            if (token->type == TokenType::Keyword && token->value == "fn") {
                parseFn();
            } else if (token->type == TokenType::Keyword && token->value == "data") {
                parseData();
            } else if (token->type == TokenType::Keyword && token->value == "struct") {
                parseStruct();
            } else {
                _context->emitError("Unexpected token '" + token->value + "' at line " + std::to_string(token->line) + ", column " + std::to_string(token->column) + " at top level. Expected 'fn', 'data', or 'struct'.");
                return;
            }
        }
    }

    Token * IRParser::peekToken(size_t offset) {
        if (_currentTokenIndex + offset >= _tokens.size()) {
            return nullptr;
        }
        return &_tokens[_currentTokenIndex + offset];
    }

    Token * IRParser::getToken() {
        if (_currentTokenIndex >= _tokens.size()) {
            return nullptr;
        }
        return &_tokens[_currentTokenIndex++];
    }
} // Oxy