#include "Tokenizer.h"

namespace Oxy {
    std::vector<Token> Tokenizer::tokenize() {
        std::vector<Token> tokens;

        while (peek() != '\0') {
            skipWhitespace();
            if (peek() == '\0') break;
            if (peek() == '#') {
                while (peek() != '\n' && peek() != '\0') {
                    get();
                }
                continue;
            }

            size_t tokenLine = _line;
            size_t tokenColumn = _column;

            if (peek() == '.' && peek(1) == '.' && peek(2) == '.') {
                get(); get(); get(); // Consume "..."
                tokens.emplace_back(TokenType::Syntax, "...", tokenLine, tokenColumn);
            } else if (isalpha(peek()) || peek() == '_' || peek() == '.') {
                std::string identifier = consumeIdentifier();
                TokenType type = TokenType::Identifier;

                // Check for keywords
                if (identifier == "fn" || identifier == "data" || identifier == "struct") {
                    type = TokenType::Keyword;
                }

                tokens.emplace_back(type, std::move(identifier), tokenLine, tokenColumn);
            } else if (isdigit(peek()) || (peek() == '-' && isdigit(peek(1)))) {
                tokens.emplace_back(TokenType::Number, consumeNumber(), tokenLine, tokenColumn);
            } else if (peek() == '"') {
                tokens.emplace_back(TokenType::String, consumeString(), tokenLine, tokenColumn);
            } else if (peek() == '@') {
                get(); // Consume '@'
                tokens.emplace_back(TokenType::Block, consumeIdentifier(), tokenLine, tokenColumn);
            } else if (peek() == '$') {
                get(); // Consume '$'
                tokens.emplace_back(TokenType::Global, consumeIdentifier(), tokenLine, tokenColumn);
            } else if (peek() == '%') {
                get(); // Consume '%'
                tokens.emplace_back(TokenType::Local, consumeIdentifier(), tokenLine, tokenColumn);
            } else if (peek() == ':') {
                get(); // Consume ':'
                tokens.emplace_back(TokenType::TypeDefinition, consumeIdentifier(), tokenLine, tokenColumn);
            } else {
                // Syntax characters
                char c = get();

                if (c == '-' && peek() == '>') {
                    get(); // Consume '>'
                    tokens.emplace_back(TokenType::Syntax, "->", tokenLine, tokenColumn);
                } else {
                    std::string value(1, c);
                    tokens.emplace_back(TokenType::Syntax, std::move(value), tokenLine, tokenColumn);
                }
            }
        }

        return tokens;
    }

    char Tokenizer::peek(int offset) {
        if (size_t(_currentIndex + offset) >= _input.size()) {
            return '\0'; // End of input
        }
        return _input[_currentIndex + offset];
    }

    char Tokenizer::get() {
        if (size_t(_currentIndex) >= _input.size()) {
            return '\0'; // End of input
        }
        char c = _input[_currentIndex++];
        if (c == '\n') {
            _line++;
            _column = 1;
        } else {
            _column++;
        }
        return c;
    }

    void Tokenizer::skipWhitespace() {
        while (isspace(peek())) {
            get();
        }
    }

    std::string_view Tokenizer::consumeWhile(std::function<bool(char)> condition) {
        size_t start = _currentIndex;
        while (condition(peek())) {
            get();
        }
        return {_input.c_str() + start, _currentIndex - start};
    }

    std::string Tokenizer::consumeIdentifier() {
        return std::string(consumeWhile([](char c) {
            return isalnum(c) || c == '_' || c == '.';
        }));
    }

    std::string Tokenizer::consumeNumber() {
        return std::string(consumeWhile([](char c) {
            return isdigit(c) || c == '.' || c == 'x' || c == 'X' || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
        }));
    }

    std::string Tokenizer::consumeString() {
        std::string result;
        get(); // Consume the opening quote
        while (peek() != '"' && peek() != '\0') {
            if (peek() == '\\') {
                get(); // Consume the backslash
                char escaped = get(); // Get the escaped character
                switch (escaped) {
                    case 'n': result += '\n'; break;
                    case 't': result += '\t'; break;
                    case '\\': result += '\\'; break;
                    case '0': result += '\0'; break;
                    case '"': result += '"'; break;
                    default: result += escaped; break; // Unknown escape, just add it
                }
            } else {
                result += get();
            }
        }
        get(); // Consume the closing quote
        return result;
    }
} // Oxy