#include "ReliefHub.h"

#include "Exceptions.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <utility>

namespace ReliefOps {
namespace Logistics {
namespace {

template <typename Number>
bool readNumber(const std::string& prompt, Number& value) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) return false;
    std::istringstream parser(line);
    Number parsed{};
    if (!(parser >> parsed) || (parser >> std::ws && !parser.eof())) {
        std::cout << "Invalid number; please enter one value in the requested format.\n";
        return false;
    }
    value = parsed;
    return true;
}

bool readText(const std::string& prompt, std::string& value) {
    std::cout << prompt;
    if (!std::getline(std::cin, value)) return false;
    if (value.empty()) {
        std::cout << "This field cannot be empty.\n";
        return false;
    }
    return true;
}

bool readYesNo(const std::string& prompt, bool& value) {
    std::string answer;
    if (!readText(prompt, answer)) return false;
    if (answer == "y" || answer == "Y" || answer == "yes" || answer == "YES") {
        value = true;
        return true;
    }
    if (answer == "n" || answer == "N" || answer == "no" || answer == "NO") {
        value = false;
        return true;
    }
    std::cout << "Enter y or n.\n";
    return false;
}

} // namespace

ReliefHub::ReliefHub() : depot_("Central Simulated Relief Depot") {
    seedData();
}

void ReliefHub::seedData() {
    depot_ += Inventory::ReliefCargo("WATER", "Drinking water packs", 1.0, 120, false);
    depot_ += Inventory::ReliefCargo("MED-KIT", "Medical kits", 2.5, 40, true);
    depot_ += Inventory::ReliefCargo("BLANKET", "Thermal blankets", 1.2, 75, false);
    fleet_.push_back(std::make_unique<AerialTransport>("AIR-01", 180.0, 450.0));
    fleet_.push_back(std::make_unique<GroundConvoy>("GROUND-01", 900.0, 700.0));
    fleet_.push_back(std::make_unique<AmphibiousCraft>("AMPH-01", 420.0, 300.0));
    fleet_.push_back(std::make_unique<HybridReliefVehicle>("HYBRID-01", 600.0, 500.0));
}

void ReliefHub::printMenu() const {
    std::cout << "\n=== ERL-HUB | Simulated Relief Logistics ===\n"
              << "1. View inventory\n"
              << "2. Add/restock supplies\n"
              << "3. Create and enqueue a mission request\n"
              << "4. Show next prioritized request\n"
              << "5. Dispatch next request\n"
              << "6. View fleet and network metrics\n"
              << "7. View manifest/audit history\n"
              << "8. Save audit history\n"
              << "9. Load audit history\n"
              << "0. Exit\n";
}

void ReliefHub::run() {
    std::cout << "ERL-HUB is an offline classroom simulation. All incident and stock data are fictional.\n";
    while (true) {
        printMenu();
        int choice = -1;
        if (!readNumber("Choice: ", choice)) {
            if (!std::cin) return;
            continue;
        }
        try {
            switch (choice) {
            case 1: showInventory(); break;
            case 2: addRestock(); break;
            case 3: createMission(); break;
            case 4:
                if (queue_.empty()) {
                    std::cout << "No mission requests are queued.\n";
                } else {
                    std::cout << "Requests in dispatch order:\n";
                    const auto ordered = queue_.orderedEntries();
                    std::for_each(ordered.begin(), ordered.end(), [](const auto& entry) {
                        std::cout << "  " << entry.value.id << " | " << entry.value.location
                                  << " | priority " << entry.priority
                                  << (entry.urgentMedical ? " | URGENT MEDICAL" : "") << '\n';
                    });
                }
                break;
            case 5: dispatchNext(); break;
            case 6: showFleet(); break;
            case 7: showHistory(); break;
            case 8: saveHistory(); break;
            case 9: loadHistory(); break;
            case 0:
                std::cout << "Simulation closed. No real emergency data were used.\n";
                return;
            default: std::cout << "Choose a listed menu number.\n"; break;
            }
        } catch (const std::exception& error) {
            std::cout << "Action failed: " << error.what() << '\n';
        }
    }
}

void ReliefHub::showInventory() const {
    std::cout << "\nInventory at " << depot_.name() << ":\n";
    std::cout << std::left << std::setw(12) << "Cargo ID" << std::setw(26) << "Supply"
              << std::setw(10) << "Units" << std::setw(14) << "Unit kg" << "Type\n";
    std::for_each(depot_.inventory().begin(), depot_.inventory().end(),
        [](const Inventory::ReliefCargo& cargo) {
        std::cout << std::left << std::setw(12) << cargo.id() << std::setw(26) << cargo.name()
                  << std::setw(10) << cargo.quantity() << std::setw(14) << cargo.unitWeightKg()
                  << (cargo.isMedical() ? "Medical" : "General") << '\n';
        });
}

void ReliefHub::showFleet() const {
    std::cout << "\nFleet status:\n";
    for (const auto& vehicle : fleet_) {
        std::cout << "  " << vehicle->id() << " | " << vehicle->type() << " | "
                  << (vehicle->operational() ? "operational" : "unavailable")
                  << " | payload " << vehicle->payloadKg() << '/' << vehicle->capacityKg()
                  << " kg | range " << vehicle->fuelRangeKm() << " km\n";
    }
    std::cout << "Operational vehicles: " << ReliefVehicle::operationalVehicleCount()
              << " | cumulative dispatched: " << ReliefVehicle::totalDispatchedTonnage()
              << " tonnes\n";
}

void ReliefHub::showHistory() const {
    if (history_.empty()) {
        std::cout << "No successful dispatch records in this session.\n";
        return;
    }
    std::cout << "\nPersistent audit/transit records:\n";
    std::cout << manifestTable;
    std::cout << std::left << std::setw(14) << "Manifest"
              << std::setw(12) << "Vehicle"
              << std::setw(12) << "Mission"
              << std::setw(24) << "Cargo"
              << std::setw(8) << "Units"
              << std::setw(12) << "Km" << "Status\n";
    for (const auto& record : history_) {
        std::cout << std::left << std::setw(14) << record.manifestId
                  << std::setw(12) << record.vehicleId
                  << std::setw(12) << record.missionId
                  << std::setw(24) << record.cargoName
                  << std::setw(8) << record.quantity
                  << std::setw(12) << record.distanceKm << record.status << '\n';
    }
}

void ReliefHub::addRestock() {
    std::string id;
    std::string name;
    double weight = 0.0;
    int quantity = 0;
    bool medical = false;
    if (!readText("Cargo ID: ", id) || !readText("Supply name: ", name) ||
        !readNumber("Unit weight in kg: ", weight) ||
        !readNumber("Quantity to add: ", quantity) ||
        !readYesNo("Medical supply? (y/n): ", medical)) return;
    if (quantity <= 0 || !std::isfinite(weight) || weight <= 0.0) {
        std::cout << "Weight and restock quantity must be positive.\n";
        return;
    }
    depot_ += Inventory::ReliefCargo(id, name, weight, quantity, medical);
    std::cout << "Restocked " << quantity << " unit(s) of " << id << ".\n";
}

void ReliefHub::createMission() {
    std::string location;
    std::string cargoId;
    int quantity = 0;
    int priority = 0;
    double distance = 0.0;
    bool urgentMedical = false;
    bool damagedCorridor = false;
    if (!readText("Incident/mission location: ", location) ||
        !readText("Cargo ID (see inventory): ", cargoId) ||
        !readNumber("Requested units: ", quantity) ||
        !readNumber("Priority (1=normal, 5=highest): ", priority) ||
        !readNumber("Route distance in km: ", distance) ||
        !readYesNo("Urgent medical need? (y/n): ", urgentMedical) ||
        !readYesNo("Damaged corridor? (y/n): ", damagedCorridor)) return;
    if (quantity <= 0 || priority < 1 || priority > 5 ||
        !std::isfinite(distance) || distance <= 0.0) {
        throw ValidationException("Quantity, priority, and route distance are outside valid limits.");
    }
    const auto* stock = depot_.find(cargoId);
    if (stock == nullptr) throw ValidationException("No stock matches cargo ID " + cargoId + ".");
    if (quantity > stock->quantity()) {
        throw ValidationException("Requested quantity exceeds current depot stock.");
    }
    if (urgentMedical && !stock->isMedical()) {
        throw ValidationException("Urgent-medical priority requires medical cargo.");
    }
    std::ostringstream requestId;
    requestId << "MIS-" << std::setw(4) << std::setfill('0') << nextMissionNumber_;
    MissionRequest request{requestId.str(), location, cargoId, quantity, priority,
                           distance, urgentMedical, damagedCorridor};
    queue_.enqueue(request, priority, urgentMedical);
    ++nextMissionNumber_;
    std::cout << "Mission " << request.id << " queued. Medical urgency and priority determine dispatch order.\n";
}

void ReliefHub::dispatchNext() {
    if (queue_.empty()) {
        std::cout << "No mission requests are queued.\n";
        return;
    }
    const MissionRequest request = queue_.next().value;
    const auto* stock = depot_.find(request.cargoId);
    if (stock == nullptr || request.quantity > stock->quantity()) {
        throw ValidationException("Dispatch stopped: requested supply is no longer available.");
    }
    const Inventory::ReliefCargo cargo(request.cargoId, stock->name(),
                                       stock->unitWeightKg(), request.quantity,
                                       stock->isMedical());
    const double cargoWeight = ReliefVehicle::calculatePayloadKg(cargo);

    ReliefVehicle* selected = nullptr;
    std::string routeFailure = "No operational vehicle can serve this route.";
    bool capacityBlocked = false;
    bool routeEvaluated = false;
    for (const auto& candidate : fleet_) {
        if (!candidate->operational()) continue;
        if (candidate->payloadKg() + cargoWeight > candidate->capacityKg()) {
            capacityBlocked = true;
            continue;
        }
        routeEvaluated = true;
        try {
            (void)candidate->routeEvaluation(request.distanceKm, request.damagedCorridor);
            selected = candidate.get();
            break;
        } catch (const std::exception& error) {
            routeFailure = error.what();
        }
    }
    if (selected == nullptr) {
        if (capacityBlocked && !routeEvaluated) {
            throw CargoOverweightException("Cargo exceeds every operational vehicle's remaining capacity.");
        }
        throw ValidationException(routeFailure);
    }

    std::ostringstream manifestId;
    manifestId << "MAN-" << std::setw(4) << std::setfill('0') << nextManifestNumber_;
    const std::string id = manifestId.str();
    for (const auto& prior : history_) {
        if (prior.manifestId == id) throw ValidationException("Duplicate manifest identity detected.");
    }

    DistributionDepot stagedDepot = depot_;
    PriorityDispatchQueue<MissionRequest> stagedQueue = queue_;
    (void)stagedQueue.pop();
    const auto dispatchedCargo = stagedDepot.allocate(
        request.cargoId, request.quantity, request.urgentMedical);
    DispatchManifest manifest(id);
    manifest.add(dispatchedCargo);
    AuditRecord record{id, selected->id(), request.id, dispatchedCargo.name(),
                       dispatchedCargo.quantity(), request.distanceKm, "DISPATCHED"};
    std::vector<AuditRecord> stagedHistory = history_;
    stagedHistory.push_back(record);

    // All fallible validations and allocations precede the live state update.
    *selected -= dispatchedCargo;
    depot_ = std::move(stagedDepot);
    history_.swap(stagedHistory);
    queue_.swap(stagedQueue);
    ++nextManifestNumber_;

    std::cout << "Dispatch successful: " << request.id << " -> " << selected->id()
              << " (" << selected->type() << "), route " << request.distanceKm << " km.\n";
    std::cout << "Manifest " << manifest.id() << ":\n" << manifestTable;
    for (const auto& item : manifest.entries()) {
        std::cout << std::left << std::setw(14) << item.id() << std::setw(24) << item.name()
                  << std::setw(10) << item.quantity() << std::setw(14) << item.totalWeightKg()
                  << '\n';
    }
}

void ReliefHub::saveHistory() {
    std::string error;
    if (!PersistenceManager::save("erl_hub_audit.bin", history_, error)) {
        throw ValidationException(error);
    }
    std::cout << "Saved " << history_.size() << " audit record(s) to erl_hub_audit.bin.\n";
}

void ReliefHub::loadHistory() {
    std::vector<AuditRecord> loaded;
    std::string error;
    if (!PersistenceManager::load("erl_hub_audit.bin", loaded, error)) {
        throw ValidationException(error);
    }
    history_.swap(loaded);
    std::cout << "Loaded " << history_.size() << " audit record(s) from erl_hub_audit.bin.\n";
}

} // namespace Logistics
} // namespace ReliefOps
