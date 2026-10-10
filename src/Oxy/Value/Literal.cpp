#include "Literal.h"

namespace Oxy {
    Literal::Literal(Type* type, Data data) {
        _type = type;
        _data = data;
    }

    Type* Literal::getType() const {
        return _type;
    }

    const Literal::Data& Literal::getData() const {
        return _data;
    }
} // Oxy