#include "DispatchManifest.h"

#include "Exceptions.h"

#include <algorithm>
#include <iomanip>
#include <istream>
#include <ostream>
#include <utility>

namespace ReliefOps {
namespace Logistics {

DispatchManifest::DispatchManifest(std::string id) : id_(std::move(id)) {
    if (id_.empty()) {
        throw ValidationException("Manifest ID cannot be empty.");
    }
}

double DispatchManifest::totalWeightKg() const noexcept {
    double weight = 0.0;
    for (const auto& cargo : entries_) weight += cargo.totalWeightKg();
    return weight;
}

bool DispatchManifest::contains(const Inventory::ReliefCargo& cargo) const noexcept {
    return std::find(entries_.begin(), entries_.end(), cargo) != entries_.end();
}

void DispatchManifest::add(const Inventory::ReliefCargo& cargo) {
    if (cargo.quantity() <= 0) {
        throw ValidationException("Manifest cargo quantity must be positive.");
    }
    if (contains(cargo)) {
        throw ValidationException("Duplicate cargo identity cannot be added to a manifest.");
    }
    entries_.push_back(cargo);
}

void DispatchManifest::swap(DispatchManifest& other) noexcept {
    id_.swap(other.id_);
    entries_.swap(other.entries_);
}

bool DispatchManifest::operator==(const DispatchManifest& other) const noexcept {
    return id_ == other.id_;
}

std::ostream& operator<<(std::ostream& output, const DispatchManifest& manifest) {
    output << std::quoted(manifest.id()) << ' ' << manifest.entries().size();
    for (const auto& cargo : manifest.entries()) output << ' ' << cargo;
    return output;
}

std::istream& operator>>(std::istream& input, DispatchManifest& manifest) {
    std::string id;
    std::size_t count = 0;
    if (!(input >> std::quoted(id) >> count) || id.empty() || count > 1000) {
        input.setstate(std::ios::failbit);
        return input;
    }
    try {
        DispatchManifest parsed(std::move(id));
        for (std::size_t index = 0; index < count; ++index) {
            Inventory::ReliefCargo cargo;
            if (!(input >> cargo)) return input;
            parsed.add(cargo);
        }
        manifest.swap(parsed);
    } catch (const std::exception&) {
        input.setstate(std::ios::failbit);
    }
    return input;
}

std::ostream& manifestTable(std::ostream& output) {
    return output << std::left << std::setw(14) << "Cargo ID"
                  << std::setw(24) << "Supply"
                  << std::setw(10) << "Units"
                  << std::setw(14) << "Weight (kg)" << '\n'
                  << std::string(62, '-') << '\n';
}

} // namespace Logistics
} // namespace ReliefOps
