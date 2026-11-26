#include "ReliefVehicle.h"

#include "Exceptions.h"

#include <cmath>
#include <utility>

namespace ReliefOps {
namespace Logistics {

double ReliefVehicle::totalDispatchedTonnage_ = 0.0;
int ReliefVehicle::operationalVehicleCount_ = 0;

GPSBeacon::GPSBeacon(std::string region) : region_(std::move(region)) {}
EmergencyLifeSupportUnit::EmergencyLifeSupportUnit(bool installed) noexcept
    : installed_(installed) {}

ReliefVehicle::ReliefVehicle(std::string id, double capacityKg, double fuelRangeKm)
    : gpsBeacon_("Simulated earthquake zone"), lifeSupport_(true),
      id_(std::move(id)), capacityKg_(capacityKg), fuelRangeKm_(fuelRangeKm) {
    if (id_.empty() || !std::isfinite(capacityKg_) || capacityKg_ <= 0.0 ||
        !std::isfinite(fuelRangeKm_) || fuelRangeKm_ <= 0.0) {
        throw ValidationException("Vehicle ID, capacity, and fuel range must be valid and positive.");
    }
    ++operationalVehicleCount_;
}

ReliefVehicle::ReliefVehicle(const ReliefVehicle& other)
    : gpsBeacon_(other.gpsBeacon_), lifeSupport_(other.lifeSupport_),
      id_(other.id_), capacityKg_(other.capacityKg_), payloadKg_(other.payloadKg_),
      fuelRangeKm_(other.fuelRangeKm_), operational_(other.operational_),
      hasBeacon_(other.hasBeacon_), hasLifeSupport_(other.hasLifeSupport_) {
    if (operational_) ++operationalVehicleCount_;
}

ReliefVehicle& ReliefVehicle::operator=(const ReliefVehicle& other) {
    if (this == &other) return *this;
    if (operational_ != other.operational_) {
        operationalVehicleCount_ += other.operational_ ? 1 : -1;
    }
    gpsBeacon_ = other.gpsBeacon_;
    lifeSupport_ = other.lifeSupport_;
    id_ = other.id_;
    capacityKg_ = other.capacityKg_;
    payloadKg_ = other.payloadKg_;
    fuelRangeKm_ = other.fuelRangeKm_;
    operational_ = other.operational_;
    hasBeacon_ = other.hasBeacon_;
    hasLifeSupport_ = other.hasLifeSupport_;
    return *this;
}

ReliefVehicle::~ReliefVehicle() {
    if (operational_) --operationalVehicleCount_;
}

void ReliefVehicle::setOperational(bool operational) noexcept {
    if (operational_ == operational) return;
    operational_ = operational;
    operationalVehicleCount_ += operational ? 1 : -1;
}

void ReliefVehicle::setRouteEquipment(bool hasBeacon, bool hasLifeSupport) noexcept {
    hasBeacon_ = hasBeacon;
    hasLifeSupport_ = hasLifeSupport;
}

double ReliefVehicle::routeBonus() const noexcept {
    return (hasBeacon_ ? 2.0 : 0.0) + (hasLifeSupport_ ? 1.0 : 0.0);
}

ReliefVehicle& ReliefVehicle::operator-=(const Inventory::ReliefCargo& cargo) {
    const double departureWeight = cargo.totalWeightKg();
    if (cargo.quantity() <= 0 || !std::isfinite(departureWeight) ||
        payloadKg_ + departureWeight > capacityKg_) {
        throw CargoOverweightException("Cargo exceeds the vehicle's remaining payload capacity.");
    }
    payloadKg_ += departureWeight;
    recordDispatch(departureWeight);
    return *this;
}

double ReliefVehicle::totalDispatchedTonnage() noexcept {
    return totalDispatchedTonnage_;
}

int ReliefVehicle::operationalVehicleCount() noexcept {
    return operationalVehicleCount_;
}

void ReliefVehicle::recordDispatch(double kilograms) noexcept {
    totalDispatchedTonnage_ += kilograms / 1000.0;
}

AerialTransport::AerialTransport(std::string id, double capacityKg, double fuelRangeKm)
    : ReliefVehicle(std::move(id), capacityKg, fuelRangeKm) {
    setRouteEquipment(true, true);
}

std::string AerialTransport::type() const { return "Aerial transport"; }

double AerialTransport::routeEvaluation(double distanceKm, bool damagedCorridor) const {
    if (!std::isfinite(distanceKm) || distanceKm <= 0.0) {
        throw ValidationException("Route distance must be a positive finite value.");
    }
    if (damagedCorridor) return distanceKm + 1000.0;
    if (distanceKm > fuelRangeKm()) {
        throw FuelDepletionException("Aerial route is beyond the vehicle's simulated fuel range.");
    }
    return distanceKm - routeBonus();
}

bool AerialTransport::supportsDamagedCorridor() const noexcept { return true; }
std::unique_ptr<ReliefVehicle> AerialTransport::clone() const {
    return std::make_unique<AerialTransport>(*this);
}

GroundConvoy::GroundConvoy(std::string id, double capacityKg, double fuelRangeKm)
    : ReliefVehicle(std::move(id), capacityKg, fuelRangeKm) {
    setRouteEquipment(true, false);
}

std::string GroundConvoy::type() const { return "Ground convoy"; }

double GroundConvoy::routeEvaluation(double distanceKm, bool damagedCorridor) const {
    if (!std::isfinite(distanceKm) || distanceKm <= 0.0) {
        throw ValidationException("Route distance must be a positive finite value.");
    }
    if (damagedCorridor) {
        throw DamagedCorridorException("Ground convoy route is blocked by a damaged corridor.");
    }
    if (distanceKm > fuelRangeKm()) {
        throw FuelDepletionException("Ground route is beyond the vehicle's simulated fuel range.");
    }
    return distanceKm - routeBonus();
}

bool GroundConvoy::supportsDamagedCorridor() const noexcept { return false; }
std::unique_ptr<ReliefVehicle> GroundConvoy::clone() const {
    return std::make_unique<GroundConvoy>(*this);
}

AmphibiousCraft::AmphibiousCraft(std::string id, double capacityKg, double fuelRangeKm)
    : ReliefVehicle(std::move(id), capacityKg, fuelRangeKm) {
    setRouteEquipment(true, true);
}

std::string AmphibiousCraft::type() const { return "Amphibious craft"; }

double AmphibiousCraft::routeEvaluation(double distanceKm, bool damagedCorridor) const {
    if (!std::isfinite(distanceKm) || distanceKm <= 0.0) {
        throw ValidationException("Route distance must be a positive finite value.");
    }
    if (damagedCorridor && !hasLifeSupport()) {
        throw DamagedCorridorException("Craft lacks required life-support equipment.");
    }
    if (distanceKm > fuelRangeKm()) {
        throw FuelDepletionException("Amphibious route is beyond the vehicle's simulated fuel range.");
    }
    return distanceKm - routeBonus() - (damagedCorridor ? 3.0 : 0.0);
}

bool AmphibiousCraft::supportsDamagedCorridor() const noexcept { return true; }
std::unique_ptr<ReliefVehicle> AmphibiousCraft::clone() const {
    return std::make_unique<AmphibiousCraft>(*this);
}

HybridReliefVehicle::HybridReliefVehicle(std::string id, double capacityKg, double fuelRangeKm)
    : ReliefVehicle(id, capacityKg, fuelRangeKm),
      AerialTransport(id, capacityKg, fuelRangeKm),
      AmphibiousCraft(std::move(id), capacityKg, fuelRangeKm) {}

std::string HybridReliefVehicle::type() const { return "Hybrid air-amphibious"; }

double HybridReliefVehicle::routeEvaluation(double distanceKm, bool damagedCorridor) const {
    if (!std::isfinite(distanceKm) || distanceKm <= 0.0) {
        throw ValidationException("Route distance must be a positive finite value.");
    }
    if (distanceKm > fuelRangeKm()) {
        throw FuelDepletionException("Hybrid route is beyond the vehicle's simulated fuel range.");
    }
    return distanceKm - routeBonus() - (damagedCorridor ? 4.0 : 0.0);
}

bool HybridReliefVehicle::supportsDamagedCorridor() const noexcept { return true; }
std::unique_ptr<ReliefVehicle> HybridReliefVehicle::clone() const {
    return std::make_unique<HybridReliefVehicle>(*this);
}

} // namespace Logistics
} // namespace ReliefOps
