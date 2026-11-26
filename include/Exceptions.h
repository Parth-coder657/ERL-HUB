#pragma once

#include <stdexcept>
#include <string>

namespace ReliefOps {
namespace Logistics {

class CargoOverweightException : public std::runtime_error {
public:
    explicit CargoOverweightException(const std::string& message)
        : std::runtime_error(message) {}
};

class FuelDepletionException : public std::runtime_error {
public:
    explicit FuelDepletionException(const std::string& message)
        : std::runtime_error(message) {}
};

class DamagedCorridorException : public std::runtime_error {
public:
    explicit DamagedCorridorException(const std::string& message)
        : std::runtime_error(message) {}
};

class ValidationException : public std::runtime_error {
public:
    explicit ValidationException(const std::string& message)
        : std::runtime_error(message) {}
};

} // namespace Logistics
} // namespace ReliefOps
