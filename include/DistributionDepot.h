#pragma once

#include "ReliefCargo.h"

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace ReliefOps {
namespace Logistics {

class IncidentCommander;

class DistributionDepot {
public:
    explicit DistributionDepot(std::string name = "Central Depot");

    const std::string& name() const noexcept { return name_; }
    const std::vector<Inventory::ReliefCargo>& inventory() const noexcept;
    Inventory::ReliefCargo& at(std::size_t index);
    const Inventory::ReliefCargo& at(std::size_t index) const;
    const Inventory::ReliefCargo* find(const std::string& cargoId) const noexcept;
    Inventory::ReliefCargo* find(const std::string& cargoId) noexcept;

    DistributionDepot& operator+=(const Inventory::ReliefCargo& restock);
    DistributionDepot& operator-=(const Inventory::ReliefCargo& departure);
    Inventory::ReliefCargo allocate(const std::string& cargoId,
                                    double requestedVolumeCubicMeters);
    Inventory::ReliefCargo allocate(const std::string& cargoId,
                                    int requestedUnits, bool urgentMedical);

private:
    friend class IncidentCommander;
    std::string name_;
    std::vector<Inventory::ReliefCargo> inventory_;
};

class IncidentCommander {
public:
    static Inventory::ReliefCargo emergencyTriageOverride(
        DistributionDepot& depot, const std::string& cargoId, int requestedUnits);
};

std::ostream& operator<<(std::ostream& output, const DistributionDepot& depot);
std::istream& operator>>(std::istream& input, DistributionDepot& depot);

} // namespace Logistics
} // namespace ReliefOps
