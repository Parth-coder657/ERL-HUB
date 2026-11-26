#pragma once

#include "ReliefCargo.h"

#include <iosfwd>
#include <memory>
#include <string>

namespace ReliefOps {
namespace Logistics {

class GPSBeacon {
public:
    explicit GPSBeacon(std::string region = "Simulated region");
    const std::string& region() const noexcept { return region_; }
private:
    std::string region_;
};

class EmergencyLifeSupportUnit {
public:
    explicit EmergencyLifeSupportUnit(bool installed = true) noexcept;
    bool installed() const noexcept { return installed_; }
private:
    bool installed_;
};

class ReliefVehicle {
public:
    ReliefVehicle(std::string id, double capacityKg, double fuelRangeKm);
    ReliefVehicle(const ReliefVehicle& other);
    ReliefVehicle& operator=(const ReliefVehicle& other);
    virtual ~ReliefVehicle();

    const std::string& id() const noexcept { return id_; }
    double capacityKg() const noexcept { return capacityKg_; }
    double payloadKg() const noexcept { return payloadKg_; }
    double fuelRangeKm() const noexcept { return fuelRangeKm_; }
    static inline double calculatePayloadKg(const Inventory::ReliefCargo& cargo) noexcept {
        return cargo.totalWeightKg();
    }
    static inline double estimateRouteDistanceKm(double directDistanceKm,
                                                  double detourFactor) noexcept {
        return directDistanceKm * detourFactor;
    }
    bool operational() const noexcept { return operational_; }
    void setOperational(bool operational) noexcept;
    virtual std::string type() const = 0;
    virtual double routeEvaluation(double distanceKm, bool damagedCorridor) const = 0;
    virtual bool supportsDamagedCorridor() const noexcept = 0;

    ReliefVehicle& operator-=(const Inventory::ReliefCargo& cargo);
    virtual std::unique_ptr<ReliefVehicle> clone() const = 0;
    static double totalDispatchedTonnage() noexcept;
    static int operationalVehicleCount() noexcept;
    static void recordDispatch(double kilograms) noexcept;

protected:
    void setRouteEquipment(bool hasBeacon, bool hasLifeSupport) noexcept;
    double routeBonus() const noexcept;
    bool hasLifeSupport() const noexcept { return hasLifeSupport_; }
    GPSBeacon gpsBeacon_;
    EmergencyLifeSupportUnit lifeSupport_;

private:
    std::string id_;
    double capacityKg_;
    double payloadKg_ = 0.0;
    double fuelRangeKm_;
    bool operational_ = true;
    bool hasBeacon_ = false;
    bool hasLifeSupport_ = false;
    static double totalDispatchedTonnage_;
    static int operationalVehicleCount_;
};

class AerialTransport : public virtual ReliefVehicle {
public:
    AerialTransport(std::string id, double capacityKg, double fuelRangeKm);
    std::string type() const override;
    double routeEvaluation(double distanceKm, bool damagedCorridor) const override;
    bool supportsDamagedCorridor() const noexcept override;
    std::unique_ptr<ReliefVehicle> clone() const override;
};

class GroundConvoy : public virtual ReliefVehicle {
public:
    GroundConvoy(std::string id, double capacityKg, double fuelRangeKm);
    std::string type() const override;
    double routeEvaluation(double distanceKm, bool damagedCorridor) const override;
    bool supportsDamagedCorridor() const noexcept override;
    std::unique_ptr<ReliefVehicle> clone() const override;
};

class AmphibiousCraft : public virtual ReliefVehicle {
public:
    AmphibiousCraft(std::string id, double capacityKg, double fuelRangeKm);
    std::string type() const override;
    double routeEvaluation(double distanceKm, bool damagedCorridor) const override;
    bool supportsDamagedCorridor() const noexcept override;
    std::unique_ptr<ReliefVehicle> clone() const override;
};

class HybridReliefVehicle final : public AerialTransport, public AmphibiousCraft {
public:
    HybridReliefVehicle(std::string id, double capacityKg, double fuelRangeKm);
    std::string type() const override;
    double routeEvaluation(double distanceKm, bool damagedCorridor) const override;
    bool supportsDamagedCorridor() const noexcept override;
    std::unique_ptr<ReliefVehicle> clone() const override;
};

} // namespace Logistics
} // namespace ReliefOps
