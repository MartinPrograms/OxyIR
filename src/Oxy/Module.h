#ifndef OXYIR_MODULE_H
#define OXYIR_MODULE_H

#include <cstring>
#include <vector>
#include <string>
#include <memory>
#include <map>

#include "Context.h"
#include "Value/Data.h"

#include "Value/Function.h"

namespace Oxy {
    class Module {
    public:
        enum class Architecture {
            Amd64
        };

        inline static std::map<Architecture, size_t> pointerSizes = {
            {Architecture::Amd64, 8}
        };

        Module(Context* context, Architecture arch, std::string name);

        [[nodiscard]] Context* getContext() const;
        [[nodiscard]] Architecture getArchitecture() const;
        [[nodiscard]] const std::string& getName() const;
        [[nodiscard]] const std::vector<std::unique_ptr<Function>>& getFunctions() const;
        [[nodiscard]] const std::vector<std::unique_ptr<Data>>& getGlobalVariables() const;

        Function* addFunction(FunctionType* type, std::string name, uint32_t flags = Function::Flags::None) {
            auto func = std::make_unique<Function>(_context, type, std::move(name), flags);
            auto funcPtr = func.get();
            _functions.emplace_back(std::move(func));
            return funcPtr;
        }

        Function* addFunction(std::unique_ptr<Function> func) {
            auto funcPtr = func.get();
            _functions.emplace_back(std::move(func));
            return funcPtr;
        }

        Data* addGlobalVariable(Type* type, std::string name, Data::Flags flags = Data::Flags::None, size_t alignment = 0, std::vector<uint8_t> initialValue = {}) {
            auto data = std::make_unique<Data>(type, std::move(name), flags, alignment, std::move(initialValue));
            auto dataPtr = data.get();
            _data.emplace_back(std::move(data));
            return dataPtr;
        }

        Data* addGlobalVariable(std::unique_ptr<Data> data) {
            auto dataPtr = data.get();
            _data.emplace_back(std::move(data));
            return dataPtr;
        }

    private:
        Context* _context;
        Architecture _arch;
        std::string _name;
        std::vector<std::unique_ptr<Function>> _functions;
        std::vector<std::unique_ptr<Data>> _data;
    };
} // Oxy

#endif //OXYIR_MODULE_H