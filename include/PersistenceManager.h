#pragma once

#include "DispatchManifest.h"

#include <string>
#include <vector>

namespace ReliefOps {
namespace Logistics {

struct AuditRecord {
    std::string manifestId;
    std::string vehicleId;
    std::string missionId;
    std::string cargoName;
    int quantity = 0;
    double distanceKm = 0.0;
    std::string status;
};

class PersistenceManager {
public:
    static bool save(const std::string& path, const std::vector<AuditRecord>& records,
                     std::string& error);
    static bool load(const std::string& path, std::vector<AuditRecord>& records,
                     std::string& error);
};

} // namespace Logistics
} // namespace ReliefOps
