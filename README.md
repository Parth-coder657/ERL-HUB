# ERL-HUB: Post-Earthquake Multi-Agency Emergency Relief Logistics Hub

**Course:** Object-Oriented Programming Using C++ (CIC-211)  
**Institution:** Maharaja Agrasen Institute of Technology  
**Student:** PARTH RAJPAL · **Enrollment:** 00114802725 · **Group:** C1

ERL-HUB is an offline, menu-driven C++17 classroom simulation. Its fictional
inventory, incident requests, fleet, dispatch manifests, and audit records are
for demonstrating object-oriented programming only. It is not an operational
emergency-response system and does not use real emergency or personal data.

## Features

- Simulated supply inventory, validated restocking, incident requests, and a
  priority queue that promotes urgent medical requests.
- Dispatch selection across aerial, ground, amphibious, and hybrid vehicles;
  route/fuel/capacity checks; manifest and audit history.
- Versioned binary audit persistence with bounded records and a temporary-file
  replacement strategy.
- Encapsulation, namespaces, function/operator overloading, safe references,
  static metrics, virtual inheritance, composition, runtime polymorphism,
  exceptions, STL containers/algorithms, a template queue, and RAII manifests.
- No paid APIs, network services, database, GUI, or third-party dependencies.

## Build and run

The project is configured for C++17. On Windows with `g++` on `PATH`, run in
PowerShell from the project directory:

```powershell
.\build.ps1
.\build\erl_hub.exe
```

To compile and immediately launch:

```powershell
.\build.ps1 -Run
```

With CMake 3.16 or newer:

```powershell
cmake -S . -B build
cmake --build build
.\build\erl_hub.exe
```

The program creates `erl_hub_audit.bin` in its current working directory only
when the user chooses **Save audit history**. The file is local runtime data
and is excluded from Git.

## Menu workflow

Start with **View inventory**, create one or more mission requests, and select
**Show next prioritized request** to inspect queue ordering. **Dispatch next
request** selects a suitable operational vehicle, deducts available stock,
and creates a manifest/audit record. Save and load audit history to exercise
binary persistence. For example, a priority-1 urgent medical request is chosen
before a priority-5 non-medical request.

## Project map

- `include/` — domain types, queue template, exceptions, and public interfaces.
- `src/` — domain implementations, persistence, menu workflow, and `main`.
- `docs/PROJECT_REPORT.md` — requirements mapping, design notes, build details,
  limitations, and verification log.
- `docs/REPORT_EXPORT.md` — instructions for exporting the report to PDF.
- `tests/smoke_test.ps1` — scripted menu, priority, validation, and persistence
  checks.
- `scripts/create_source_archive.ps1` — builds a clean source ZIP outside the
  source tree, with a 10 MB limit.

## Limitations

All dispatch rules, route scores, distances, quantities, and vehicle
capabilities are teaching examples, not engineering or emergency guidance.
The demonstration has one in-memory depot, a session-local mission queue,
simulated vehicle ranges, and simple audit records. It does not model live
communications, maps, real stock systems, staffing, safety protocols, or
multi-user coordination.
