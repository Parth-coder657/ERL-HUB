#include "DistributionDepot.h"

#include "Exceptions.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <istream>
#include <limits>
#include <ostream>
#include <utility>

namespace ReliefOps {
namespace Logistics {

DistributionDepot::DistributionDepot(std::string name) : name_(std::move(name)) {
    if (name_.empty()) {
        throw ValidationException("A depot must have a name.");
    }
}

const std::vector<Inventory::ReliefCargo>& DistributionDepot::inventory() const noexcept {
    return inventory_;
}

Inventory::ReliefCargo& DistributionDepot::at(std::size_t index) {
    return inventory_.at(index);
}

const Inventory::ReliefCargo& DistributionDepot::at(std::size_t index) const {
    return inventory_.at(index);
}

const Inventory::ReliefCargo* DistributionDepot::find(const std::string& cargoId) const noexcept {
    const auto item = std::find_if(inventory_.begin(), inventory_.end(),
        [&cargoId](const Inventory::ReliefCargo& cargo) { return cargo.id() == cargoId; });
    return item == inventory_.end() ? nullptr : &*item;
}

Inventory::ReliefCargo* DistributionDepot::find(const std::string& cargoId) noexcept {
    const auto item = std::find_if(inventory_.begin(), inventory_.end(),
        [&cargoId](const Inventory::ReliefCargo& cargo) { return cargo.id() == cargoId; });
    return item == inventory_.end() ? nullptr : &*item;
}

DistributionDepot& DistributionDepot::operator+=(const Inventory::ReliefCargo& restock) {
    if (restock.id().empty() || restock.quantity() <= 0) {
        throw ValidationException("Restock must contain a valid cargo ID and positive quantity.");
    }
    auto* existing = find(restock.id());
    if (existing != nullptr) {
        if (existing->name() != restock.name() ||
            existing->unitWeightKg() != restock.unitWeightKg() ||
            existing->isMedical() != restock.isMedical()) {
            throw ValidationException("A cargo ID cannot be restocked with conflicting item details.");
        }
        if (restock.quantity() > std::numeric_limits<int>::max() - existing->quantity()) {
            throw ValidationException("Restock quantity exceeds the supported limit.");
        }
        existing->setQuantity(existing->quantity() + restock.quantity());
    } else {
        inventory_.push_back(restock);
    }
    return *this;
}

DistributionDepot& DistributionDepot::operator-=(const Inventory::ReliefCargo& departure) {
    auto* existing = find(departure.id());
    if (existing == nullptr || departure.quantity() <= 0 ||
        departure.quantity() > existing->quantity()) {
        throw ValidationException("Departure cargo is not available in the requested quantity.");
    }
    *existing -= departure.quantity();
    return *this;
}

Inventory::ReliefCargo DistributionDepot::allocate(
    const std::string& cargoId, double requestedVolumeCubicMeters) {
    if (!std::isfinite(requestedVolumeCubicMeters) || requestedVolumeCubicMeters <= 0.0) {
        throw ValidationException("Requested volume must be a positive finite value.");
    }
    const auto* stock = find(cargoId);
    if (stock == nullptr) {
        throw ValidationException("Unknown cargo ID: " + cargoId);
    }
    constexpr double cubicMetersPerUnit = 0.01;
    const double units = std::floor(requestedVolumeCubicMeters / cubicMetersPerUnit);
    if (units < 1.0 || units > stock->quantity()) {
        throw ValidationException("Requested volume is too small or exceeds available stock.");
    }
    Inventory::ReliefCargo allocation(cargoId, stock->name(), stock->unitWeightKg(),
                                      static_cast<int>(units), stock->isMedical());
    *this -= allocation;
    return allocation;
}

Inventory::ReliefCargo DistributionDepot::allocate(
    const std::string& cargoId, int requestedUnits, bool urgentMedical) {
    if (requestedUnits <= 0) {
        throw ValidationException("Requested units must be positive.");
    }
    const auto* stock = find(cargoId);
    if (stock == nullptr) {
        throw ValidationException("Unknown cargo ID: " + cargoId);
    }
    if (urgentMedical && !stock->isMedical()) {
        throw ValidationException("Urgent-medical allocation is reserved for medical cargo.");
    }
    if (requestedUnits > stock->quantity()) {
        throw ValidationException("Requested quantity exceeds available stock.");
    }
    Inventory::ReliefCargo allocation(cargoId, stock->name(), stock->unitWeightKg(),
                                      requestedUnits, stock->isMedical());
    *this -= allocation;
    return allocation;
}

Inventory::ReliefCargo IncidentCommander::emergencyTriageOverride(
    DistributionDepot& depot, const std::string& cargoId, int requestedUnits) {
    if (requestedUnits <= 0) {
        throw ValidationException("Emergency triage quantity must be positive.");
    }
    const auto item = std::find_if(depot.inventory_.begin(), depot.inventory_.end(),
        [&cargoId](const Inventory::ReliefCargo& cargo) { return cargo.id() == cargoId; });
    if (item == depot.inventory_.end() || !item->isMedical() ||
        requestedUnits > item->quantity()) {
        throw ValidationException("Triage override requires available medical cargo only.");
    }
    Inventory::ReliefCargo allocation(cargoId, item->name(), item->unitWeightKg(),
                                      requestedUnits, true);
    *item -= requestedUnits;
    return allocation;
}

std::ostream& operator<<(std::ostream& output, const DistributionDepot& depot) {
    output << std::quoted(depot.name()) << ' ' << depot.inventory().size() << '\n';
    for (const auto& cargo : depot.inventory()) {
        output << cargo << '\n';
    }
    return output;
}

std::istream& operator>>(std::istream& input, DistributionDepot& depot) {
    std::string name;
    std::size_t count = 0;
    if (!(input >> std::quoted(name) >> count) || count > 10000) {
        input.setstate(std::ios::failbit);
        return input;
    }
    try {
        DistributionDepot parsed(std::move(name));
        for (std::size_t index = 0; index < count; ++index) {
            Inventory::ReliefCargo cargo;
            if (!(input >> cargo)) return input;
            parsed += cargo;
        }
        depot = std::move(parsed);
    } catch (const std::exception&) {
        input.setstate(std::ios::failbit);
    }
    return input;
}

} // namespace Logistics
} // namespace ReliefOps
