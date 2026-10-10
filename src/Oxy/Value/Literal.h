#ifndef OXYIR_LITERAL_H
#define OXYIR_LITERAL_H

#include "Value.h"
#include <variant>

namespace Oxy {
    class Literal : public Value {
    public:
        using Data = std::variant<int64_t, double>;
        explicit Literal(Type* type, Data data);
        ~Literal() override = default;

        [[nodiscard]] Type* getType() const override;
        [[nodiscard]] const Data& getData() const;

    private:
        Type* _type;
        Data _data;
    };
} // Oxy

#endif //OXYIR_LITERAL_H