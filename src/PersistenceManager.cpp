#include "PersistenceManager.h"

#include "Exceptions.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <unordered_set>
#include <utility>

namespace ReliefOps {
namespace Logistics {
namespace {

constexpr std::uint32_t fileVersion = 1;
constexpr std::uint32_t maximumRecords = 10000;
constexpr std::uint32_t maximumStringLength = 4096;
const char fileSignature[4] = {'E', 'R', 'L', 'H'};

bool validRecord(const AuditRecord& record) {
    return !record.manifestId.empty() && record.manifestId.size() <= maximumStringLength &&
           !record.vehicleId.empty() && record.vehicleId.size() <= maximumStringLength &&
           !record.missionId.empty() && record.missionId.size() <= maximumStringLength &&
           !record.cargoName.empty() && record.cargoName.size() <= maximumStringLength &&
           record.quantity > 0 && std::isfinite(record.distanceKm) &&
           record.distanceKm > 0.0 && !record.status.empty() &&
           record.status.size() <= 64;
}

bool writeU32(std::ostream& output, std::uint32_t value) {
    for (unsigned int shift = 0; shift < 32; shift += 8) {
        output.put(static_cast<char>((value >> shift) & 0xFFU));
    }
    return static_cast<bool>(output);
}

bool readU32(std::istream& input, std::uint32_t& value) {
    value = 0;
    for (unsigned int shift = 0; shift < 32; shift += 8) {
        const int byte = input.get();
        if (byte == std::char_traits<char>::eof()) return false;
        value |= static_cast<std::uint32_t>(static_cast<unsigned char>(byte)) << shift;
    }
    return true;
}

bool writeString(std::ostream& output, const std::string& value) {
    if (value.empty() || value.size() > maximumStringLength ||
        !writeU32(output, static_cast<std::uint32_t>(value.size()))) {
        return false;
    }
    output.write(value.data(), static_cast<std::streamsize>(value.size()));
    return static_cast<bool>(output);
}

bool readString(std::istream& input, std::string& value) {
    std::uint32_t length = 0;
    if (!readU32(input, length) || length == 0 || length > maximumStringLength) {
        return false;
    }
    std::string parsed(length, '\0');
    input.read(&parsed[0], static_cast<std::streamsize>(length));
    if (!input) return false;
    value = std::move(parsed);
    return true;
}

bool writeRecord(std::ostream& output, const AuditRecord& record) {
    return writeString(output, record.manifestId) &&
           writeString(output, record.vehicleId) &&
           writeString(output, record.missionId) &&
           writeString(output, record.cargoName) &&
           writeU32(output, static_cast<std::uint32_t>(record.quantity)) &&
           [&output, &record]() {
               std::uint64_t distanceBits = 0;
               static_assert(sizeof(distanceBits) == sizeof(record.distanceKm),
                             "Binary double size is unsupported.");
               std::memcpy(&distanceBits, &record.distanceKm, sizeof(distanceBits));
               for (unsigned int shift = 0; shift < 64; shift += 8) {
                   output.put(static_cast<char>((distanceBits >> shift) & 0xFFU));
               }
               return static_cast<bool>(output);
           }() &&
           writeString(output, record.status);
}

bool readRecord(std::istream& input, AuditRecord& record) {
    AuditRecord parsed;
    std::uint32_t quantity = 0;
    std::uint64_t distanceBits = 0;
    if (!readString(input, parsed.manifestId) ||
        !readString(input, parsed.vehicleId) ||
        !readString(input, parsed.missionId) ||
        !readString(input, parsed.cargoName) ||
        !readU32(input, quantity) || quantity == 0 ||
        quantity > static_cast<std::uint32_t>(std::numeric_limits<int>::max())) {
        return false;
    }
    for (unsigned int shift = 0; shift < 64; shift += 8) {
        const int byte = input.get();
        if (byte == std::char_traits<char>::eof()) return false;
        distanceBits |= static_cast<std::uint64_t>(static_cast<unsigned char>(byte)) << shift;
    }
    std::memcpy(&parsed.distanceKm, &distanceBits, sizeof(parsed.distanceKm));
    if (!std::isfinite(parsed.distanceKm) || parsed.distanceKm <= 0.0 ||
        !readString(input, parsed.status) || parsed.status.size() > 64) {
        return false;
    }
    parsed.quantity = static_cast<int>(quantity);
    record = std::move(parsed);
    return true;
}

} // namespace

bool PersistenceManager::save(const std::string& path,
                              const std::vector<AuditRecord>& records,
                              std::string& error) {
    error.clear();
    if (path.empty() || records.size() > maximumRecords) {
        error = "Save path is empty or record count exceeds the file limit.";
        return false;
    }
    std::unordered_set<std::string> manifestIds;
    for (const auto& record : records) {
        if (!validRecord(record) || !manifestIds.insert(record.manifestId).second) {
            error = "Audit records contain invalid fields or duplicate manifest IDs.";
            return false;
        }
    }
    const std::filesystem::path destination(path);
    const std::filesystem::path temporary = destination.string() + ".tmp";
    const std::filesystem::path backup = destination.string() + ".bak";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        if (!output) {
            error = "Could not open the temporary audit file for writing.";
            return false;
        }
        output.write(fileSignature, sizeof(fileSignature));
        const bool headerOk = writeU32(output, fileVersion) &&
            writeU32(output, static_cast<std::uint32_t>(records.size()));
        bool recordsOk = headerOk;
        for (const auto& record : records) {
            if (!recordsOk || !writeRecord(output, record)) {
                recordsOk = false;
                break;
            }
        }
        output.flush();
        if (!recordsOk || !output) {
            output.close();
            std::error_code cleanupError;
            std::filesystem::remove(temporary, cleanupError);
            error = "Writing the temporary audit file failed; the prior save was preserved.";
            return false;
        }
    }

    std::error_code renameError;
    std::filesystem::rename(temporary, destination, renameError);
    if (!renameError) return true;

    std::error_code existsError;
    const bool destinationExists = std::filesystem::exists(destination, existsError);
    if (existsError || !destinationExists) {
        error = "Could not replace the audit file: " + renameError.message();
        return false;
    }

    std::error_code backupError;
    std::filesystem::remove(backup, backupError);
    backupError.clear();
    std::filesystem::rename(destination, backup, backupError);
    if (backupError) {
        error = "Could not preserve the previous audit file: " + backupError.message();
        return false;
    }
    renameError.clear();
    std::filesystem::rename(temporary, destination, renameError);
    if (renameError) {
        std::error_code restoreError;
        std::filesystem::rename(backup, destination, restoreError);
        error = "Could not install the new audit file: " + renameError.message();
        if (restoreError) error += " Previous-save restore also failed: " + restoreError.message();
        return false;
    }
    std::error_code removeError;
    std::filesystem::remove(backup, removeError);
    return true;
}

bool PersistenceManager::load(const std::string& path,
                              std::vector<AuditRecord>& records,
                              std::string& error) {
    error.clear();
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        error = "Could not open the audit file.";
        return false;
    }
    char signature[sizeof(fileSignature)]{};
    input.read(signature, sizeof(signature));
    std::uint32_t version = 0;
    std::uint32_t count = 0;
    if (!input || !std::equal(std::begin(fileSignature), std::end(fileSignature),
                              std::begin(signature)) ||
        !readU32(input, version) || version != fileVersion ||
        !readU32(input, count) || count > maximumRecords) {
        error = "Audit file header is corrupt, unsupported, or truncated.";
        return false;
    }
    std::vector<AuditRecord> parsed;
    parsed.reserve(count);
    std::unordered_set<std::string> manifestIds;
    for (std::uint32_t index = 0; index < count; ++index) {
        AuditRecord record;
        if (!readRecord(input, record) || !validRecord(record) ||
            !manifestIds.insert(record.manifestId).second) {
            error = "Audit file contains an invalid, duplicate, or truncated record.";
            return false;
        }
        parsed.push_back(std::move(record));
    }
    if (input.peek() != std::char_traits<char>::eof()) {
        error = "Audit file contains unexpected trailing data.";
        return false;
    }
    records.swap(parsed);
    return true;
}

} // namespace Logistics
} // namespace ReliefOps
