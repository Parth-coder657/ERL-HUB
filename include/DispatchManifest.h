#pragma once

#include "ReliefCargo.h"

#include <iosfwd>
#include <string>
#include <vector>

namespace ReliefOps {
namespace Logistics {

class DispatchManifest {
public:
    explicit DispatchManifest(std::string id);
    DispatchManifest(const DispatchManifest&) = default;
    DispatchManifest& operator=(const DispatchManifest&) = default;
    DispatchManifest(DispatchManifest&&) noexcept = default;
    DispatchManifest& operator=(DispatchManifest&&) noexcept = default;
    ~DispatchManifest() = default;

    const std::string& id() const noexcept { return id_; }
    const std::vector<Inventory::ReliefCargo>& entries() const noexcept { return entries_; }
    double totalWeightKg() const noexcept;
    bool contains(const Inventory::ReliefCargo& cargo) const noexcept;
    void add(const Inventory::ReliefCargo& cargo);
    void swap(DispatchManifest& other) noexcept;
    bool operator==(const DispatchManifest& other) const noexcept;

private:
    std::string id_;
    std::vector<Inventory::ReliefCargo> entries_;
};

std::ostream& operator<<(std::ostream& output, const DispatchManifest& manifest);
std::istream& operator>>(std::istream& input, DispatchManifest& manifest);
std::ostream& manifestTable(std::ostream& output);

} // namespace Logistics
} // namespace ReliefOps
