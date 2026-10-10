#ifndef OXYIR_FUNCTION_H
#define OXYIR_FUNCTION_H

#include "Value.h"
#include "../Type/FunctionType.h"
#include "Oxy/BasicBlock.h"

namespace Oxy {
    class Function : public Value {
    public:
        enum Flags : uint32_t {
            None     = 0,
            Export   = 1 << 0,
            Inline   = 1 << 1,
            Extern   = 1 << 2,
        };

        Function(Context* context, FunctionType* type, std::string name, uint32_t flags = Flags::None);
        ~Function() override = default;

        [[nodiscard]] Type* getType() const override;
        [[nodiscard]] const std::string& getName() const;
        [[nodiscard]] const std::vector<std::unique_ptr<BasicBlock>>& getBasicBlocks() const;
        [[nodiscard]] uint32_t getFlags() const;
        [[nodiscard]] bool isExtern() const;

        BasicBlock* addBlock(std::unique_ptr<BasicBlock> block) {
            if (_flags & Flags::Extern) {
                _context->emitError("Cannot add basic block to an external function");
            }

            if (_blocks.empty()) _entry = block.get();
            return _blocks.emplace_back(std::move(block)).get();
        }

        BasicBlock* addBlock(std::string name) {
            if (_flags & Flags::Extern) {
                _context->emitError("Cannot add basic block to an external function");
            }

            auto block = std::make_unique<BasicBlock>(_context, std::move(name));
            if (_blocks.empty()) _entry = block.get();
            return _blocks.emplace_back(std::move(block)).get();
        }

        BasicBlock* addEntryBlock() {
            if (_flags & Flags::Extern) {
                _context->emitError("Cannot add basic block to an external function");
            }

            if (_entry) {
                throw std::runtime_error("Entry block already exists");
            }
            return addBlock("entry");
        }

        [[nodiscard]] BasicBlock* getEntry() const { return _entry; }
        [[nodiscard]] const std::vector<std::unique_ptr<BasicBlock>>& getBlocks() const { return _blocks; }
        [[nodiscard]] const std::vector<std::unique_ptr<Value>>& getArguments() const { return _arguments; }

    private:
        Context * _context;
        FunctionType* _type;
        std::string _name;
        uint32_t _flags;
        BasicBlock* _entry = nullptr;
        std::vector<std::unique_ptr<BasicBlock>> _blocks;
        std::vector<std::unique_ptr<Value>> _arguments;
    };
} // Oxy

#endif //OXYIR_FUNCTION_H