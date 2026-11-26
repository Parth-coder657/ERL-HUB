# Post-Earthquake Multi-Agency Emergency Relief Logistics Hub (ERL-HUB)

## Project cover

| Field | Details |
|---|---|
| Course | Object-Oriented Programming Using C++ (CIC-211) |
| Institution | Maharaja Agrasen Institute of Technology |
| Student name | PARTH RAJPAL |
| Enrollment number | 00114802725 |
| Class group | C1 |
| Language / standard | C++17 |
| Project type | Offline, menu-driven console simulation |

> **Academic-use and safety note:** ERL-HUB is a fictional classroom simulation
> of supply allocation and vehicle dispatch. It is not an operational
> emergency-response system, and it must not be used to coordinate real
> incidents or make real-world safety decisions.

## Abstract

ERL-HUB models a small relief-logistics workflow after a fictional earthquake.
The program keeps sample supplies at a depot, accepts incident mission
requests, orders them by priority and urgent medical need, selects an available
vehicle based on route and payload constraints, and records successful
dispatches. Users can inspect the fleet and inventory, restock supplies, and
save or reload audit records in a bounded, versioned binary file. The project
uses C++17 and the standard library to demonstrate course concepts including
encapsulation, function and operator overloading, inheritance, virtual
dispatch, exceptions, STL containers, RAII, and stream I/O.

## Problem statement

An introductory program can represent a response workflow using related
objects rather than unrelated values and procedural steps. This project
provides a small, testable simulation in which inventory constraints, mission
urgency, vehicle capabilities, and an audit trail interact. It focuses on
teaching object-oriented design and does not model actual disaster operations.

## Objectives

- Represent cargo, depot stock, missions, vehicles, manifests, and audit
  records as cohesive types.
- Validate user input and reject invalid quantities, priorities, stock
  requests, duplicate identities, unsuitable routes, and excessive payloads.
- Demonstrate dynamic binding through a fleet stored as
  `std::unique_ptr<ReliefVehicle>`.
- Use exception-safe staging for dispatch and validated binary persistence.
- Provide a readable menu workflow and reproducible local build/test commands.

## Scope and limitations

All seed records, cargo weights, vehicle ranges, route outcomes, and missions
are invented examples. The simulation has one in-memory depot, a session-local
priority queue, a small fixed demonstration fleet, simple route checks, and
local audit persistence. It has no network, map service, database, GUI,
multi-user coordination, live stock integration, emergency protocols, or
real-world calibration. Route scoring and vehicle capabilities are teaching
rules only.

## System design

### Architecture and modules

| Module | Responsibility |
|---|---|
| `include/ReliefCargo.h`, `src/ReliefCargo.cpp` | Validated supply identity, unit weight, quantity, medical flag, and stream operators. |
| `include/DistributionDepot.h`, `src/DistributionDepot.cpp` | Encapsulated inventory, restock/departure operators, allocation overloads, and emergency triage access. |
| `include/ReliefVehicle.h`, `src/ReliefVehicle.cpp` | Polymorphic vehicles, virtual-inheritance types, route evaluation, composition, payload, and fleet metrics. |
| `include/DispatchManifest.h`, `src/DispatchManifest.cpp` | Dispatch manifest identity, cargo-entry validation, formatted I/O, and RAII storage. |
| `include/PriorityDispatchQueue.h` | Reusable template queue, stable sequence ordering, urgent-medical functor, and duplicate request checks. |
| `include/PersistenceManager.h`, `src/PersistenceManager.cpp` | Versioned and bounded binary audit serialization, validation, and temporary-file replacement. |
| `include/Exceptions.h` | Domain-specific errors used by cargo, vehicle, and route operations. |
| `include/ReliefHub.h`, `src/ReliefHub.cpp`, `src/main.cpp` | Seed data, menu/input workflow, dispatch transaction, and application entry point. |

### Principal classes

- `ReliefOps::Inventory::ReliefCargo` represents one supply type and its current
  quantity.
- `ReliefOps::Logistics::DistributionDepot` owns cargo objects and controls
  validated restock and departure operations.
- `ReliefOps::Logistics::ReliefVehicle` is the abstract fleet base. Its derived
  types are `AerialTransport`, `GroundConvoy`, `AmphibiousCraft`, and
  `HybridReliefVehicle`.
- `GPSBeacon` and `EmergencyLifeSupportUnit` are member objects composed by
  `ReliefVehicle`; they are not base classes.
- `DispatchManifest` owns its entries in a `std::vector`. Explicitly defaulted
  copy/move operations and destructor demonstrate the Rule of Five while
  delegating memory ownership to RAII.
- `PriorityDispatchQueue<T>` orders requests with an internal comparison
  functor and rejects duplicate request identities.
- `ReliefHub` connects the menu, depot, queue, fleet, manifests, and audit
  records.

### Menu and demonstration workflow

The menu can show stock, restock an item, create and enqueue a mission, inspect
the current dispatch order, dispatch the next mission, view fleet metrics and
audit history, save/load history, and exit. A useful demonstration is to add a
normal priority-5 water request and an urgent priority-1 medical request. The
medical request is selected first despite its lower numeric priority. A
subsequent sufficiently heavy cargo request skips the smaller aerial vehicle
and can use the ground convoy if its route is suitable. The smoke test also
submits an over-stock request and verifies that it is rejected.

## Requirements-to-implementation map

### Unit I — Core foundations and call semantics

| Requirement | Implementation |
|---|---|
| I.1 Logical namespaces `ReliefOps::Logistics` and `ReliefOps::Inventory` | Domain declarations and implementations in `include/ReliefCargo.h`, `include/DistributionDepot.h`, `include/ReliefVehicle.h`, and corresponding `src/*.cpp` files. |
| I.2 Two meaningful supply-allocation overloads | `DistributionDepot::allocate(const std::string&, double)` converts requested volume to units; `DistributionDepot::allocate(const std::string&, int, bool)` validates a unit request and medical urgency. See `src/DistributionDepot.cpp`. |
| I.3 Call by reference and safe return by reference | Reference parameters are used by `readNumber`, `readText`, `PersistenceManager::save/load`, and depot operators. `DistributionDepot::at` provides const/non-const references to owned vector elements, with standard vector invalidation when the depot is modified. See `include/DistributionDepot.h`, `src/ReliefHub.cpp`, and `include/PersistenceManager.h`. |
| I.4 Inline distance/payload utilities | `ReliefVehicle::calculatePayloadKg` and `ReliefVehicle::estimateRouteDistanceKm` are inline in `include/ReliefVehicle.h`; payload calculation is used by dispatch in `src/ReliefHub.cpp`. |
| I.5 Narrow friend triage override | `IncidentCommander` is a friend of `DistributionDepot`; `emergencyTriageOverride` directly examines the private inventory but only permits a validated quantity of existing medical cargo. See `include/DistributionDepot.h` and `src/DistributionDepot.cpp`. |

### Unit II — Encapsulation, dynamic memory, and operators

| Requirement | Implementation |
|---|---|
| II.1 Encapsulated cargo, depot, vehicle classes | Private state, validating constructors/operations, and const-correct accessors in `include/ReliefCargo.h`, `include/DistributionDepot.h`, and `include/ReliefVehicle.h`. |
| II.2 Static network metrics | `ReliefVehicle::totalDispatchedTonnage`, `operationalVehicleCount`, `recordDispatch`, and their static data members in `src/ReliefVehicle.cpp`. |
| II.3 Dynamic manifest memory and Rule of Five / RAII | `DispatchManifest` owns a dynamic `std::vector<ReliefCargo>` and explicitly defaults copy construction, copy assignment, move construction, move assignment, and destruction in `include/DispatchManifest.h`. No raw owning pointer is used. |
| II.4 `+=`, `-=`, `==`, `<<`, and `>>` | Depot `+=` restocks and depot `-=` deducts departures; cargo `-=` deducts a positive quantity; cargo equality compares cargo IDs; manifest equality compares manifest IDs and `add` blocks duplicate cargo IDs. Cargo, depot, and manifest stream operators are implemented in `src/ReliefCargo.cpp`, `src/DistributionDepot.cpp`, and `src/DispatchManifest.cpp`. Duplicate manifest IDs are checked before dispatch in `src/ReliefHub.cpp`. |

### Unit III — Inheritance, containment, and polymorphism

| Requirement | Implementation |
|---|---|
| III.1 Polymorphic vehicle hierarchy | Abstract `ReliefVehicle` plus `AerialTransport`, `GroundConvoy`, and `AmphibiousCraft` in `include/ReliefVehicle.h` and `src/ReliefVehicle.cpp`. |
| III.2 Diamond inheritance using a virtual base | `AerialTransport` and `AmphibiousCraft` virtually inherit `ReliefVehicle`; `HybridReliefVehicle` derives from both and initializes one shared base subobject. See `include/ReliefVehicle.h` and its constructor in `src/ReliefVehicle.cpp`. |
| III.3 Composition/containment | `GPSBeacon` and `EmergencyLifeSupportUnit` are `ReliefVehicle` member objects in `include/ReliefVehicle.h`; neither is inherited. |
| III.4 Base-pointer storage, dynamic dispatch, safe destruction | `ReliefHub::fleet_` stores `std::unique_ptr<ReliefVehicle>`. `routeEvaluation` and `type` are virtual, and `ReliefVehicle` has a virtual destructor. |
| III.5 Runtime fleet routing | `ReliefHub::dispatchNext` calls virtual `routeEvaluation` through the selected `ReliefVehicle*` and chooses an eligible simulated vehicle. The runtime override depends on the concrete vehicle type (dynamic binding). |

### Unit IV — Streams, persistence, exception safety, and STL

| Requirement | Implementation |
|---|---|
| IV.1 Domain exception classes | `CargoOverweightException`, `FuelDepletionException`, `DamagedCorridorException`, and `ValidationException` derive from `std::runtime_error` in `include/Exceptions.h`. |
| IV.2 Exception-safe state updates | `ReliefHub::dispatchNext` performs route/stock checks, stages depot/queue/history/manifests, and only then updates the vehicle and live state. Cargo/depot operations validate before mutation. |
| IV.3 Binary audit persistence | `PersistenceManager::save/load` in `src/PersistenceManager.cpp` writes a signature/version, bounded record count, length-prefixed strings, numeric fields, and validated records using binary streams. It rejects duplicate manifest IDs and does not serialize object memory or pointers. It writes a `.tmp` file before renaming; on platforms that cannot replace an existing target with `rename` (notably Windows), it moves the old target to `.bak`, installs the temporary file, and attempts restoration if installation fails. This fallback preserves recoverability but is not a crash-proof, single-operation atomic replacement during the brief backup/rename interval. |
| IV.4 Template, STL, functor, and manipulator | `PriorityDispatchQueue<T>` uses `std::vector`, `std::unordered_map`, `std::sort`, and the custom `HigherPriority` comparison functor. `std::for_each` is used for inventory output in `src/ReliefHub.cpp`. `manifestTable(std::ostream&)` in `src/DispatchManifest.cpp` is a custom tabular stream manipulator. |
| IV.5 User-visible persistence demonstration | Menu actions `saveHistory` and `loadHistory` report record counts. `tests/smoke_test.ps1` executes save and reload and checks that both records are confirmed. |

## Object-oriented and implementation notes

- **Encapsulation and namespaces:** domain state is private and accessed
  through validated operations. Inventory and logistics types are separated
  into their named namespaces.
- **Overloading and references:** allocation overloads intentionally have
  different input units/semantics. The depot's element references refer to
  owned vector storage and must not be retained across an operation that can
  reallocate that vector.
- **Static members:** dispatching a cargo load adds its weight in tonnes to the
  network metric. Vehicle constructors/destructors and operational-state
  changes maintain the operational count for the in-memory fleet.
- **RAII and copy/move behavior:** the manifest's `std::vector` owns its
  allocations. Its explicitly defaulted special members preserve independent
  copies and transfer storage on moves without manual allocation or deletion.
- **Operator semantics:** `+=` means restock; depot `-=` means departure stock
  deduction; cargo `-=` deducts item units; `==` uses cargo/manifest identity;
  stream operators encode values rather than object memory.
- **Virtual inheritance and composition:** the hybrid resolves a shared
  `ReliefVehicle` base through virtual inheritance. Beacon and life-support
  objects are contained member objects, illustrating a separate
  has-a relationship.
- **Runtime polymorphism:** the dispatch loop stores base pointers and asks
  each concrete vehicle to evaluate a route. The derived override supplies
  that vehicle's simulated route rule.
- **Exceptions and strong state updates:** invalid user or domain values
  produce explicit messages. Dispatch prepares its changed depot, audit
  history, and queue before the live update; failed route/stock/capacity
  checks leave those live records unchanged. Allocation failure by the
  standard library is not converted into a success-shaped response.
- **Binary format:** the file uses a four-byte `ERLH` signature, format version
  1,   little-endian 32-bit lengths/counts, bounded strings stored as byte sequences, integer
  quantities, and an eight-byte floating-point route distance. Loading checks
  truncation, bounds, values, version, and trailing data before replacing the
  caller's records. Double representation portability is limited to platforms
  with an eight-byte compatible floating-point representation.

## Build and run

The verified compiler available in the workspace is `g++ 10.3.0`. From
PowerShell at the project root:

```powershell
.\build.ps1
.\build\erl_hub.exe
```

The `build.ps1` helper compiles the seven `.cpp` files with `-std=c++17
-Wall -Wextra -Wpedantic` into `build/erl_hub.exe`. The project also includes a
`CMakeLists.txt` for CMake 3.16+, but CMake was not available on PATH in the
verified environment. Audit data are written to the program's current working
directory only after selecting the save menu item.

## Verification log

| Command | Actual outcome |
|---|---|
| `g++ --version` | Available compiler reported `g++.exe (tdm64-1) 10.3.0`. |
| `.\build.ps1` | **PASS** — compiled all sources as C++17; no warnings or errors were printed on the successful build. |
| `.\tests\smoke_test.ps1` | **PASS** — exercised an over-stock rejection, urgent-medical queue ordering, two dispatches through the aerial vehicle, a heavy load dispatched through `GROUND-01` due to payload capacity, repeated save to an existing file, reload of all three audit records, and rejection of a deliberately truncated file. Test data and the generated binary audit file were kept in a temporary directory and removed by the test. |
| `.\scripts\create_source_archive.ps1 -OutputPath <temporary ZIP>` | **PASS** — created a 0.03 MB archive; inspection found 23 source/documentation entries and no build output, executable, IDE folder, or runtime audit data. The temporary verification ZIP was removed. |
| CMake configure/build | **NOT RUN** — CMake was not found on PATH in the inspected environment. |

The smoke test checks actual console output and the persistence file; it does
not constitute exhaustive unit testing, operational validation, or a safety
certification.

### Illustrative menu transcript

The following is an illustrative abbreviated interaction (not a verbatim
captured terminal transcript):

```text
Choice: 3
Mission location: Clinic North
Cargo ID: MED-KIT
Requested units: 4
Priority: 1
Urgent medical need? y
Mission MIS-0002 queued.

Choice: 4
Requests in dispatch order:
  MIS-0002 | Clinic North | priority 1 | URGENT MEDICAL
  MIS-0001 | Bridge Camp | priority 5

Choice: 5
Dispatch successful: MIS-0002 -> AIR-01 (Aerial transport), route 30 km.
Manifest MAN-0001 ...

Choice: 8
Saved 1 audit record(s) to erl_hub_audit.bin.
```

## Limitations and future improvements

The program is intentionally a small educational simulation. Future
course-approved extensions could add unit tests for each domain operation,
multiple depot locations, configurable simulated vehicle data, clearer route
planning examples, stronger cross-platform binary portability, and more
structured audit integrity checks. Any such extension would remain a
simulation unless separately engineered and evaluated for an appropriate
real-world use.

## Conclusion

ERL-HUB combines the course's core OOP and STL topics in one small offline
workflow: validated stock, prioritized requests, polymorphic vehicle
selection, a manifest, and persistent audit records. Its fictional data and
explicit limitations keep the project focused on programming concepts rather
than operational emergency guidance.

## References

No external sources were consulted for this implementation. Language and
library features are used from the C++17 standard library; no third-party
dependency is required.