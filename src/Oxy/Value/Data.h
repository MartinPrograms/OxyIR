#ifndef OXYIR_GLOBALVARIABLE_H
#define OXYIR_GLOBALVARIABLE_H

#include "Value.h"
#include "../Type/Type.h"
#include <vector>

namespace Oxy {
    class Data : public Value {
    public:
        enum class Flags : uint32_t {
            None = 0,
            Export = 1 << 0,
            ThreadLocal = 1 << 1,
            Mutable = 1 << 2,
            Align = 1 << 3
        };

        Data(Type* type, std::string name, Flags flags = Flags::None, size_t alignment = 0, std::vector<uint8_t> initialValue = {})
            : _pointerType(new PointerType()), _type(type), _name(std::move(name)), _flags(flags), _alignment(alignment), _initialValue(std::move(initialValue)) {}

        ~Data() override = default;

        [[nodiscard]] Type* getType() const override {
            return _pointerType;
        }

        [[nodiscard]] Type* getElementType() const {
            return _type;
        }

        [[nodiscard]] const std::string& getName() const {
            return _name;
        }

        [[nodiscard]] Flags getFlags() const {
            return _flags;
        }

        [[nodiscard]] size_t getAlignment() const {
            return _alignment;
        }

        [[nodiscard]] const std::vector<uint8_t>& getInitialValue() const {
            return _initialValue;
        }

    private:
        Type* _pointerType;
        Type* _type;
        std::string _name;
        Flags _flags;
        size_t _alignment = 0;
        std::vector<uint8_t> _initialValue;
    };
}

#endif //OXYIR_GLOBALVARIABLE_H