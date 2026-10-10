#ifndef OXYIR_TOKENIZER_H
#define OXYIR_TOKENIZER_H
#include "Context.h"

namespace Oxy {

    enum class TokenType {
        Identifier, // the instruction (alloca, stuff.xyz)
        Number, // literal number (123, 0x1A, 3.14, etc.)
        String, // "string literal"
        Block, // @block_name
        Global, // $global_name
        Local, // %local_name
        TypeDefinition, // :type_name
        Keyword, // fn, data, struct.
        Syntax, // (, ), {, }, [, ], ,, ...
        Unknown
    };

    struct Token {
        TokenType type;
        std::string value;
        size_t line;
        size_t column;

        Token(TokenType type, std::string value, size_t line, size_t column)
            : type(type), value(std::move(value)), line(line), column(column) {}
    };

    class Tokenizer {
    public:
        explicit Tokenizer(std::string input) : _input(std::move(input)) {}
        ~Tokenizer() = default;

        std::vector<Token> tokenize();

    private:
        std::string _input;
        int _currentIndex = 0;
        int _line = 1;
        int _column = 1;

        char peek(int offset = 0); // Look at the current character without consuming it. Offset allows looking ahead.
        char get(); // Consume and return the current character, advancing the position.
        void skipWhitespace();

        std::string_view consumeWhile(std::function<bool(char)> condition); // Consume characters while the condition is true and return the consumed string.
        std::string consumeIdentifier(); // Consume an identifier
        std::string consumeNumber(); // Consume a number literal
        std::string consumeString(); // Consume a string literal
    };
} // Oxy

#endif //OXYIR_TOKENIZER_H