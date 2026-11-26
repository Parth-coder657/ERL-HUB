#include "ReliefCargo.h"

#include "Exceptions.h"

#include <cmath>
#include <iomanip>
#include <istream>
#include <ostream>
#include <utility>

namespace ReliefOps {
namespace Inventory {

ReliefCargo::ReliefCargo()
    : id_("UNSPECIFIED"), name_("Unspecified supply"), unitWeightKg_(1.0) {}

ReliefCargo::ReliefCargo(std::string id, std::string name, double unitWeightKg,
                         int quantity, bool medical)
    : id_(std::move(id)), name_(std::move(name)), unitWeightKg_(unitWeightKg),
      medical_(medical) {
    if (id_.empty() || name_.empty() || !std::isfinite(unitWeightKg_) ||
        unitWeightKg_ <= 0.0 || quantity < 0) {
        throw Logistics::ValidationException("Cargo details must have names, a positive unit weight, and a non-negative quantity.");
    }
    quantity_ = quantity;
}

double ReliefCargo::totalWeightKg() const noexcept {
    return unitWeightKg_ * quantity_;
}

void ReliefCargo::setQuantity(int quantity) {
    if (quantity < 0) {
        throw Logistics::ValidationException("Cargo quantity cannot be negative.");
    }
    quantity_ = quantity;
}

ReliefCargo& ReliefCargo::operator-=(int quantity) {
    if (quantity <= 0 || quantity > quantity_) {
        throw Logistics::ValidationException("Departure quantity must be positive and available in stock.");
    }
    quantity_ -= quantity;
    return *this;
}

bool ReliefCargo::operator==(const ReliefCargo& other) const noexcept {
    return id_ == other.id_;
}

std::ostream& operator<<(std::ostream& output, const ReliefCargo& cargo) {
    return output << std::quoted(cargo.id()) << ' ' << std::quoted(cargo.name()) << ' '
                  << std::setprecision(10) << cargo.unitWeightKg() << ' '
                  << cargo.quantity() << ' ' << cargo.isMedical();
}

std::istream& operator>>(std::istream& input, ReliefCargo& cargo) {
    std::string id;
    std::string name;
    double weight = 0.0;
    int quantity = 0;
    bool medical = false;
    if (input >> std::quoted(id) >> std::quoted(name) >> weight >> quantity >> medical) {
        try {
            ReliefCargo parsed(std::move(id), std::move(name), weight, quantity, medical);
            cargo = std::move(parsed);
        } catch (const std::exception&) {
            input.setstate(std::ios::failbit);
        }
    }
    return input;
}

} // namespace Inventory
} // namespace ReliefOps
