#pragma once

#include "DistributionDepot.h"
#include "DispatchManifest.h"
#include "PersistenceManager.h"
#include "PriorityDispatchQueue.h"
#include "ReliefVehicle.h"

#include <memory>
#include <string>
#include <vector>

namespace ReliefOps {
namespace Logistics {

struct MissionRequest {
    std::string id;
    std::string location;
    std::string cargoId;
    int quantity = 0;
    int priority = 1;
    double distanceKm = 0.0;
    bool urgentMedical = false;
    bool damagedCorridor = false;

    const std::string& identity() const noexcept { return id; }
};

class ReliefHub {
public:
    ReliefHub();

    void run();
    void showInventory() const;
    void showFleet() const;
    void showHistory() const;
    void addRestock();
    void createMission();
    void dispatchNext();
    void saveHistory();
    void loadHistory();

private:
    void printMenu() const;
    void seedData();
    DistributionDepot depot_;
    PriorityDispatchQueue<MissionRequest> queue_;
    std::vector<std::unique_ptr<ReliefVehicle>> fleet_;
    std::vector<AuditRecord> history_;
    unsigned long nextMissionNumber_ = 1;
    unsigned long nextManifestNumber_ = 1;
};

} // namespace Logistics
} // namespace ReliefOps
