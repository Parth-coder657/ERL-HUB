#pragma once

#include <iosfwd>
#include <string>

namespace ReliefOps {
namespace Inventory {

class ReliefCargo {
public:
    ReliefCargo();
    ReliefCargo(std::string id, std::string name, double unitWeightKg,
                int quantity, bool medical = false);

    const std::string& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    double unitWeightKg() const noexcept { return unitWeightKg_; }
    int quantity() const noexcept { return quantity_; }
    bool isMedical() const noexcept { return medical_; }
    double totalWeightKg() const noexcept;

    void setQuantity(int quantity);
    ReliefCargo& operator-=(int quantity);
    bool operator==(const ReliefCargo& other) const noexcept;

private:
    std::string id_;
    std::string name_;
    double unitWeightKg_ = 0.0;
    int quantity_ = 0;
    bool medical_ = false;
};

std::ostream& operator<<(std::ostream& output, const ReliefCargo& cargo);
std::istream& operator>>(std::istream& input, ReliefCargo& cargo);

} // namespace Inventory
} // namespace ReliefOps
